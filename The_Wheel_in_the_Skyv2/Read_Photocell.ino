bool readPhoto(void *)
{
  PhotoValue = analogRead(sensorPin);
  PhotoValue = constrain(PhotoValue, sensorMin, sensorMax);
  PhotoValue = map(PhotoValue, sensorMin, sensorMax, 0, 100);
  
  if (debug)
  {
    Serial.print("PC after Constrain:");
    Serial.println(PhotoValue);
  }


  if (PhotoValue < dark)
  {
    allowIR = 1;
  }
  else
  {
    allowIR = 0;
  }

  return true;

}
