#include <WiFi101.h>
#include <MQTT.h>
#include <Servo.h>

// Credentials for the Wifi Network
const char ssid[] = "your_wifi_network";
const char pass[] = "your_wifi_password";

WiFiClient net;
MQTTClient client;

// pins for the two Servos (Primary and Secondary)
const int servoPrimaryPin = 2;
const int servoSecondaryPin = 3;

// Calibration for the Primary servo
const int boxPrimaryOpened = 100;
const int boxPrimaryClosed = 15;
int boxPrimaryStatus = 15;

// Calibration for the Secondary servo
const int boxSecondaryOpened = 80;
const int boxSecondaryClosed = 165;
int boxSecondaryStatus = 165;

// The two servos
Servo servoPrimary;
Servo servoSecondary;


// Establishing the WiFi Connection
void connect() {
  Serial.print( "checking wifi..." );
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print( "." );
    delay( 1000 );
  }
  Serial.print( "\nconnecting..." );
  while ( !client.connect( "arduino" , "try" , "try" ) ) {
    Serial.print( "." );
    delay( 1000 );
  }
  Serial.println( "\nconnected!" );

  // Subscription to the MQTT Topic on the broker
  client.subscribe( "box_open" );
  
}
void messageReceived( String &topic , String &payload ) {
  Serial.print( topic );
  Serial.print( ": " );
  Serial.println( payload );

  // if payload on the MQTT message is 'true' and the box is not already open, the servos open the box
  if ( payload == "true" ){
    if ( boxPrimaryStatus != boxPrimaryOpened ) {
      for ( int i = boxPrimaryClosed ; i < boxPrimaryOpened ; i++ ){
        servoPrimary.write( i );
        servoSecondary.write( boxSecondaryClosed - ( i - boxPrimaryClosed ) );
        
        // Opening speed
        delay(20);
      }
      boxPrimaryStatus = boxPrimaryOpened;
    }
  }
  // if the payload on the MQTT message is 'false' and the box is not already cloase, the servos close the box 
  if ( payload == "false" ){
    if ( boxPrimaryStatus != boxPrimaryClosed ) {
      for ( int i = boxPrimaryOpened ; i > boxPrimaryClosed ; i-- ){
        servoPrimary.write( i );
        servoSecondary.write( boxPrimaryOpened + boxSecondaryOpened - i );

        // Closing speed
        delay(20);
      }
      boxPrimaryStatus = boxPrimaryClosed;
    }
  }
}

void setup() {

  // initializing the servos and the default position (closed)
  servoPrimary.attach( servoPrimaryPin );
  servoSecondary.attach( servoSecondaryPin );
  servoPrimary.write( boxPrimaryStatus );
  servoSecondary.write( boxSecondaryStatus );

  // initializing Serial (for debug) and Wifi
  Serial.begin( 115200 );
  WiFi.begin( ssid , pass );

  // IP address of the MQTT Broker
  client.begin( "your_MQTT_broker_IP_Address" , net );
  client.onMessage( messageReceived );
  connect();
}

void loop() {
  client.loop();
  if ( !client.connected() ) {
    connect();
  }
}
