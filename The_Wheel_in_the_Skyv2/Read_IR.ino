void ReadSensor()
{
  /*
    Serial.println("Human?");
    Serial.print("Allow IR Check?:");
    Serial.print(allowIR);
    Serial.print(" | Lights off?");
    Serial.println(lightsOn);
  */

  uint8_t i = 0;
  // global defined
  ir1Last = ir2Last = ir3Last = ir4Last = 0;

  if (allowIR && !Human) {

    while (i <= 10) {
      if (movementSensor.available()) {
        ir1Last += movementSensor.getIR1();
        ir2Last += movementSensor.getIR2();
        ir3Last += movementSensor.getIR3();
        ir4Last += movementSensor.getIR4();
        movementSensor.refresh();
        i++;
      }
    }

    if (inRange(ir1Last, ir2Last, ir3Last, ir4Last, IRMin, IRMax) )
    {

      lightsOn = true;
      Human = true;
      humanCount++;
      humanTime = ((humanCount * IRtimer) / 1000) - 5;
      twinkle();
      
      if (debug)
      {
        Serial.println("in Range");
        Serial.println("Run Warmup");
        Serial.print("HumanCount: ");
        Serial.println(humanCount);
        Serial.print("HumanTime: ");
        Serial.println(humanTime);
        Serial.print("5 Sec loop: ");
        Serial.print(oneloop);
        Serial.print(" | 30 Sec loop: ");
        Serial.print(twoloop);
        Serial.print("| 60 Sec loop: ");
        Serial.print(threeloop);
        Serial.print("| 100 Sec loop: ");
        Serial.print(fourloop);
        Serial.print("| 130 Sec loop: ");
        Serial.print(fiveloop);
        Serial.print("| 160 Sec loop: ");
        Serial.println(sixloop);
      }

    }
    else
    {
      if (debug)
      {
        Serial.print("out of Range: ");
        Serial.print("| IR1:");
        Serial.print(ir1Last);
        Serial.print("| IR2:");
        Serial.print(ir2Last);
        Serial.print("| IR3:");
        Serial.print(ir3Last);
        Serial.print("| IR4:");
        Serial.println(ir3Last);
      }
      oneloop = true;
      twoloop = true;
      threeloop = true;
      fourloop = true;
      fiveloop = true;
      sixloop = true;


      lightsOn = false;
      humanCount = 0;
      humanTime = -1;
      breathing = false;

    }
  }
}
