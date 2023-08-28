void logDataOnSD() {
  digitalWrite(SS_PIN, HIGH);
  digitalWrite(SD_pin, LOW);
  // Open file
  myFile = SD.open(logPath, FILE_APPEND);
  if (myFile) {
    myFile.print(each_uid);
    myFile.print(", ");

    // Save time on SD card
    DateTime now = rtc.now();
    myFile.print((now.year()), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print(',');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.println(now.minute(), DEC);
    myFile.close();

    // Print time on Serial monitor

    //    Serial.print(now.year(), DEC);
    //    Serial.print('/');
    //    Serial.print(now.month(), DEC);
    //    Serial.print('/');
    //    Serial.print(now.day(), DEC);
    //    Serial.print(' ');
    //    Serial.print(now.hour(), DEC);
    //    Serial.print(':');
    //    Serial.println(now.minute(), DEC);
    //        Serial.println("sucessfully written on SD card");

  }
  else {
    //    Serial.println("error opening data.txt");
  }
  // Disables SD card chip select pin
  digitalWrite(SD_pin, HIGH);

}
