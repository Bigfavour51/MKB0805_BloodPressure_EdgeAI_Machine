//////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
          
           BOOT
             │
             ▼
     Initialize Hardware
             │
             ▼
       Show Splash Screen
             │
             ▼
      Connect Wi-Fi (optional)
             │
             ▼
      Waiting for Measurement
             │
             ▼
      Acquire Sensor Data
             │
             ▼
      Validate Measurement
             │
             ▼
      AI Analysis
             │
             ▼
        Store to SD
             │
             ▼
      Update OLED
             │
             ▼
       Publish MQTT
             │
             ▼
     Check Alert Condition
             │
      ┌──────┴──────┐
      ▼             ▼
   Normal        Emergency
      │             │
      ▼             ▼
 Short Beep     Alarm + Remote Alert
      │             │
      └──────┬──────┘
             ▼
          Return Idle



////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////

 Measurement Complete
        │
        ▼
Update OLED
        │
        ▼
Save to SD
        │
        ▼
Queue for Upload
        │
        ▼
Return to Idle


///////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////


Wi-Fi Available?
       │
      Yes
       │
       ▼
Upload queued records
       │
       ▼
Mark as uploaded