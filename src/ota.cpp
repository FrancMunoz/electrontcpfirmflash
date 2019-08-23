#include "ota.h"
#include "system_update.h"

void downloadFirmware() {
    TCPClient client;
    u_char buffer[512];
    u_int avail;
    u_int timeOut;
    u_int recibido=0;
    bool hayHeader=false;
    bool done=false;

    char c;
    char last=NULL;
    char *pos=0;
    char *pto=0;
    u_int length=0;
    u_int response=0;
    bool hf=false;
    u_int timePrg=0;
    bool ok=false;
    int result;

    const char *host="www.zeroworks.net";
    const char *path="/updates/ota/flash.bin";

    FileTransfer::Descriptor file;

    String header="";

    Serial.println("Downloading Firmware");

    if(Cellular.ready()) {
        if(client.connect(host, 80)) {

            IPAddress clientIP = client.remoteIP();
            Serial.print("IP: ");
            Serial.println(clientIP);

            client.print("GET ");
            client.print(path);
            client.println(" HTTP/1.0");

            client.printlnf("Host: %s", host);
            client.println("Connection: close");
            client.println("User-Agent: ZeroWorks IoT");

            client.println();
            client.flush();

            timeOut=millis();
            while(!done) {
                if(!client.status() && !client.connected()) {
                    Serial.println("Connection Error!");
                    break;
                }

                if(millis()-timeOut>5000) {
                    Serial.println("Timeout!");
                    break;
                }

                avail=client.available();
                if(avail>0) {
                    if(!hayHeader) {
                        // Leemos de uno en uno
                        c=client.read();
                        header.concat(c);

                        if(c=='\n' && last=='\r') {
                            if(hf) {
                                hayHeader=true;
                                Serial.print("Header:");
                                Serial.println(header);

                                pos=strstr(header, " ");
                                if(pos) {
                                    pos+=1;
                                    response=atoi(pos);
                                    Serial.print("Response Code: ");
                                    Serial.println(response);
                                }

                                pos=strstr(header, "Content-Length: ");
                                if(pos) {
                                    pos+=16;
                                    length=atoi(pos);
                                    Serial.print("Download Length: ");
                                    Serial.println(length);
                                }

                                if(response!=200) {
                                    Serial.println("Invalid response code.");
                                    done=true;
                                }

                                if(!done && length>0) {
                                    Serial.println("Preparing update.");
                                    file.file_length=length;
                                    file.file_address=0;
                                    file.chunk_address=0;
                                    file.chunk_size=0;
                                    file.store = FileTransfer::Store::FIRMWARE;

                                    result = Spark_Prepare_For_Firmware_Update(file, 0, NULL);
                                    if(result!=0) {
                                        Serial.printlnf("Prepare failed %d", result);
                                        done=true;
                                        break;
                                    } else {
                                        // Set Start Address returned from prepare.
                                        file.chunk_address = file.file_address;
                                        Serial.printlnf("Address: ");
                                        Serial.println(file.chunk_address);
                                    }
                                }
                            } else {
                                hf=true;
                            }
                        } else {
                            if(c!='\r') hf=false;
                        }
                        last=c;
                        timePrg=millis();
                    } else {
                        if(avail>512) avail=512;
                        avail=client.read(&buffer[0], avail);
                        recibido+=avail;

                        // Set chunk sise from downloaded data.
                        file.chunk_size=avail;
                        result = Spark_Save_Firmware_Chunk(file, &buffer[0], NULL);
                		if (result != 0) {
                			Serial.printlnf("Save chunk failed %d", result);
                			done=true;
                			break;
                		}

                        // Move start to next chunk...
                        file.chunk_address+=avail;

                        if(millis()-timePrg>1000 || recibido>=length) {
                            // Show progress each second and at the end.
                            timePrg=millis();
                            Serial.print("Progress ");
                            Serial.print(recibido);
                            Serial.print(" of ");
                            Serial.println(length);
                        }

                        if(length>0 && recibido>=length) {
                            done=true;
                            ok=true;
                            Serial.println("Firmware received.");
                        }
                    }
                    timeOut=millis();
                }
            }
            if(recibido==0) {
                Serial.println("Header:");
                Serial.println(header);
            } else {
                Serial.print("Received ");
                Serial.print(recibido);
                Serial.println(" bytes.");
            }
        } else {
            Serial.println("Unable to connect.");
        }

        client.stop();

    } else {
        Serial.println("Not connected.");
    }

    if(ok) {
        Serial.println("Launching Update");
        result = Spark_Finish_Firmware_Update(file, ok, NULL);
        if(result != 0) {
            Serial.printlnf("Finish failed %d", result);
            ok=true;
        }
    } else {
        Serial.println("Process finished with an error");
    }
}
