#include "mbed.h"
#include "EthernetNetIf.h"
#include "MQTTClient.h"
#include "ID12RFID.h"

ID12RFID rfid(p14);

EthernetNetIf ethernet;  

DigitalOut tag_present(LED1); 

#define IDS_COUNT 6
const int ids_list[IDS_COUNT] = {9656082, 4478536, 4486983, 4486994, 4469045, 89481811};
const char* names_list[IDS_COUNT] = {"yilun", "Simon", "Dan", "Mark", "Peter", "Rob"};


IpAddr serverIpAddr(10,1,1,5);  /*Server ip address*/

void callback(char* topic, char* payload);

MQTTClient mqtt(serverIpAddr, 1883, callback);

void callback(char* topic, char* payload)
{
    printf("Topic: %s\r\n", topic);
    printf("Payload: %s\r\n\r\n", payload);
    //Send incoming payloads back to topic "/mbed".
    mqtt.publish("/mbed", payload);
}

int main() {
	
    printf("\r\n############### MQTTClient Tester  #########\r\n\r\n");
    
    EthernetErr ethErr = ethernet.setup();
    if(ethErr){
        printf("Ethernet Error %d\r\n", ethErr);  
    } else {
        printf("mbed is online...\r\n");
    }
    
    char clientID[] = "mbed";   /*Client nanme show for MQTT server*/
    char topic[] = "/mbed";     /*Publish topic*/
    char sub_topic[] = "/mirror";   /*Subscribe to topic : "/mirror" */
	
    if(mqtt.connect(clientID)){
        printf("\r\nConnect to server sucessed ..\r\n");
    } else {
        printf("\r\nConnect to server failed ..\r\n");
        return -1;
    }
    
    mqtt.publish("/mbed", "Hello here is mbed...");
    wait(2);
    printf("\r\nPlease swape your card..\r\n");
    mqtt.publish("/mbed", "Please swape your card...");
    mqtt.subscribe(sub_topic);
    
    while(true) {
        //Read from first reader
        if(rfid.readable()) {
            int id = rfid.read();
            tag_present = 1;
            char temp[1024];
            for(int i = 0; i < IDS_COUNT; i++) {
                if (ids_list[i] == id){
                    printf("Tag: %d    Name:%s\r\n", id, names_list[i]);
                    sprintf(temp, "%s is at Room 412 now!", names_list[i]);
                    mqtt.publish(topic, temp);
                }
            }
            tag_present = 0;        
        }
        Net::poll();
        mqtt.live();
    }
	
    mqtt.disconnect();
    
    printf("#### End of the test.. ####\r\n\r\n");
}