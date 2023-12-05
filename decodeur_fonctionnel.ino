typedef struct Trame Trame;
int led[] = {4, 5, 6, 7, 8};
int pin = 12;
int pinLed = 13;
int j;
byte adr[15];
int p2[14] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
struct Trame
{
  long adresse = 0;
  byte vitesse;
  byte type = 0;
  byte sens ; //1:avant 0:arriere
  byte fonction[29];// = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

void setup()
{
  pinMode(pin, INPUT);
  pinMode(led[1], OUTPUT);
  pinMode(led[2], OUTPUT);
  pinMode(led[3], OUTPUT);
  pinMode(led[4], OUTPUT);
  pinMode(led[0], OUTPUT);
  pinMode(pinLed, OUTPUT);
  Serial.begin(115200);
  Serial.println("ok");
  testTrame();
}

void loop()
{
  Trame trame;
  trame = getTrame();
  if (trame.type != 255 && trame.type != 0)
  {
    //if (trame.adresse == 4)
    //{
    //  Serial.println("ok");
    //}
    Serial.print("A:");
    Serial.println(trame.adresse);
    //Serial.print("T:");
    //Serial.println(trame.type);
    Serial.print("V=");
    Serial.println(int(float(trame.vitesse)) * 255 / 31);
    //analogWrite(11, int(float(trame.vitesse) * 255 / 31));
    Serial.print("S=");
    Serial.println(boolean(trame.sens));
    //analogWrite(10, int(trame.sens) * 255);
    Serial.print("F:");
    for (j = 0; j <= 28; j++)
    {
      if (trame.fonction[j] == 1)
      {
        Serial.print("0");
      }
      else
      {
        Serial.print("1");
      }
      if (j%7 == 0)
      {
        Serial.print("\n");
      }
    }
  }
}

Trame getTrame(void)
{
  int i;
  byte bite;
  Trame trame;
  byte selector;
  byte erreur = 0;

  waitStartTrame();
  //Serial.println("dt");

  trame.adresse = getAdresse();
  //if (trame.adresse == 268)
  //{
    //Serial.println("at");
    selector = getTypeOfTrame();
    trame.type = selector;
    switch (selector)
    {
      case 0: // trame idle
        //Serial.println("idle0");
        break;
      case 1: //impossible (0001) il y a forcément un 1 dans les 3 premiers
        Serial.println("idle");
        trame.adresse = -1;
        break;
      case 2: // impossible (0010) ici on serait en 128 pas mais alors suivi que de un pour compléter l'octet
        Serial.println("s=2");
        break;
      case 3: //vitesse 128 pas
        trame.sens = getSens_128();
        trame.vitesse = getVitesse_128();
        //Serial.println("s=3");
        break;
      case 4: //14 marche arriere
        trame.sens = 0;
        trame.vitesse = getVitesse_14();
        //Serial.println("marr 14");
        //Serial.println("s=4");
        break;
      case 5: //28 marche arriere
        trame.sens = 0;
        trame.vitesse = getVitesse_28();
        //Serial.println("marr 28");
        //Serial.println("s=5");

        break;
      case 6: //14 marche avant
        trame.sens = 1;
        trame.vitesse = getVitesse_14();
        //Serial.println("mav14");
        //Serial.println("s=6");

        break;
      case 7: //28 marche avant
        trame.sens = 1;
        trame.vitesse = getVitesse_28();
        //Serial.println("mav28");
        //Serial.println("s=7");
        break;

      case 8: //f1 to f4 avec feux éteints (f0=0)
        //      //Serial.println("phare OFF");
        trame.fonction[0] = byte(0);
        //    Serial.println("s=8");// temps treeeeeeeeeeeeeeeeeeeeees limiteeeeeeeeeeeeeeeeeeeeeeeeeeee
        getF1toF4(trame.fonction);
        break;


      case 9: //f1 to f4 avec feux allumés (f0=1)
        //      //Serial.println("phare ON");
        trame.fonction[0] = 1;
        //    Serial.println("s=9"); //idem s=8
        getF1toF4(trame.fonction);
        break;




      case 10: //f5 to f8  errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrreur de stephane
        getF9toF12(trame.fonction);
        break;


      case 11: //f9 to f12 errrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrreur de stephane
        getF5toF8(trame.fonction);
        //Serial.println("s=11");
        break;


      case 12: //not yet
        Serial.println("s=12");
        break;


      case 13: //f13 to f20 ou f21 to f28 en fonction du 8 bit de l'octet
        getF13toF28(trame.fonction);
        break;


      case 14://CV
        Serial.println("s=14");
        break;


      case 15://CV
        Serial.println("idle");
        trame.adresse = -1;//mauvais à cause de byte ........
        trame.vitesse = -1;
        trame.sens = -1;
        break;

      default: //aucun des cas précédent donc erreur
        //Serial.println("erreur selector");
        break;
    }
  //}
  if (erreur == 1 || trame.vitesse == 255 || trame.adresse == -1)
  {
    trame.type = 255;
  }

  return trame;
}

byte readBit(void)
{
  byte duration;
  byte bite;
  duration = pulseIn(pin, HIGH);
  if (duration >= 40 && duration <= 100)
  {
    bite = 1;
    //Serial.print(1);
    //Serial.println(duration);
  }
  else if (duration >=100 && duration <= 140)
  {
    bite = 0;
    //Serial.print(0);
    //Serial.println(duration);
  }
  else
  {
    bite = 2;
    Serial.println(duration);
  }
  return bite;
}

void waitStartTrame()
{
  int count = 0;
  count = 0;
  while (count <= 12)
  {
    if (readBit() == 1)
    {
      count = count + 1;
    }
    else
    {
      count = 0;
    }
  }
  //Fin du préambule minimum
  while (readBit() != 0);
  //Serial.println("dept");
  //Début de la trame après le bit 0.
}

int getAdresse()
{
  int i;
  byte bite;
  int adresse = 0;


  bite = readBit();
  //Serial.print(bite);
  if (bite == 1) //adresse longue ou accessoire ou idle
  {
    //Serial.println("adresse longue ou ac");
    bite = readBit();
    //Serial.print(bite);
    if (bite == 1) //adresse longue ou idle
    { //Serial.println("al");
      for (i = 13; i >= 8 ; i--) // lecture dans le premier octet________________bit de stop
      {
        bite = readBit();
        if (bite != 2)
        {
          adresse = adresse + (long)bite * p2[i];
        }
        else
        {
          return -1; //erreur
        }
      }
      if (readBit() == 0) //bit de start
      { for (i = 7; i >= 0 ; i--) // lecture dans le deuxième octet
        {
          bite = readBit();
          if (bite != 2)
          {
            adresse = adresse + (long)bite * p2[i];
          }
          else
          {
            return -1; //erreur
          }
        }
      }
      else
      {
        return -1; //erreur
      }
    }
    else if (bite == 0) // adresse accessoire
    {
      return -1; // changer quand on implementera les accessoires.
    }
    else
    {
      return -1; //erreur
    }
    return adresse;
  }

  else if (bite == 0) //adresse courte
  {
    for (i = 6; i >= 0 ; i--)// ___________________________________pb taille (par rapport à 7===> ok
    {
      bite = readBit();
      if (bite != 2)
      {
        adresse = adresse + ((long)bite * p2[i]);
      }
      else
      {
        return -1; //erreur
      }
    }
    return adresse;
  }
  else
  {
    return -1; //erreur
  }
}


byte getTypeOfTrame(void)
{
  byte selector = 0;
  byte bite;
  int i;
  if (readBit() != 0) //verification du bit séparateur
  {
    return -1;

  }

  for (i = 3; i >= 0; i--) // lecture du type de paquet.// on pourrait prendre les 3 premier bit pour savoir
    //si il s'agit d'une 14/28 ou 128/fonction/idle
    //et rappeler dans le selector
  {
    bite = readBit();
    if (bite != 2)
    {
      selector = selector + bite * (byte)my_pow(i); //______________multiplication
    }
    else
    {
      Serial.println("bite2");
      return -1;
    }
  }
  return selector;
}



byte getVitesse_14()//_________________renomons là getVitesse_14_28
{ //if premier bite ==1 o=alors structure 28 pas sinon structure 14 pas .....
  byte bite, i, vitesse;
  //  bite = readBit();
  //  if (bite != 2)
  //  {
  //    vitesse = bite;
  //  }
  //  else
  //  {
  //    return 255;
  //  }
  vitesse = 0;
  for (i = 4; i >= 1; i--)
  {
    bite = readBit();
    if (bite != 2)
    {
      vitesse = vitesse + bite * my_pow(i);
    }
    else
    {
      return 120;
    }
  }
  return vitesse;
}

byte getVitesse_28()//_________________renomons là getVitesse_14_28
{ //if premier bite ==1 o=alors structure 28 pas sinon structure 14 pas .....
  byte bite, i, vitesse;
  //  bite = readBit();
  //  if (bite != 2)
  //  {
  //    vitesse = bite;
  //  }
  //  else
  //  {
  //    return 255;
  //  }
  vitesse = 1;
  for (i = 4; i >= 1; i--)
  {
    bite = readBit();
    if (bite != 2)
    {
      vitesse = vitesse + bite * my_pow(i);
    }
    else
    {
      return 120;
    }
  }
  return vitesse;
}



byte getSens_128()
{
  byte bite, i, sens;
  //Serial.println("ds");
  for (i = 0; i <= 3; i++)
  {
    if (readBit() != 1)
    {
      Serial.println("erreur sens");
      return 255; //erreur à dépisté
    }
  }

  if (readBit() != 0)
  {
    Serial.println("erreur sens");
    return 255;//erreur à dépisté
  }
  bite = readBit(); // sens de la loco
  sens = bite;
  return sens;
}


byte getVitesse_128()
{ byte bite, i, vitesse;
  //Serial.println("dv");
  vitesse = 0;
  //Serial.println("dv2");
  for (i = 0 ; i <= 6 ; i++)
  {
    bite = readBit();
    if (bite != 2)
    {
      vitesse = vitesse + bite * (byte)(p2[6 - i]);
      //Serial.println(int(vitesse));
    }
    else
    {
      Serial.println("errv");
      return 120;  //erreur à dépisté
    }
  }
  return vitesse;
}

void getF1toF4(byte Tab[])//_____________________attention pb avec les lampes déjà gérées===> ok
{
  byte bite;
  int i;
  for (i = 4; 1 <= i; i--)
  {
    bite = readBit();
    if (bite != 2)
    {
      Tab[i] = bite;
    }
    else
    {
      Serial.println("bp");
    }

  }
}

void getF5toF8(byte Tab[])
{
  byte bite;
  int i;
  for (i = 8; 5 <= i; i--)
  {
    bite = readBit();
    if (bite != 2)
    {
      Tab[i] = bite;
    }
    else
    {
      Serial.println("bp");
    }

  }
}

void getF9toF12(byte Tab[])
{
  byte bite;
  int i;
  for (i = 12; 9 <= i; i--)
  {
    bite = readBit();
    if (bite != 2)
    {
      Tab[i] = bite;
    }
    else
    {
      Serial.println("bp");
    }

  }
}


void getF13toF28(byte Tab[])
{
  byte bite;
  int i;
  for (i = 1; i <= 3; i++)
  {
    bite = readBit();
    //    if (bite != 1)
    //    {
    //      break;
    //    }
  }
  bite = readBit();
  if (bite == 0) //F13toF20
  {
    bite = readBit();
    if (bite == 0) //bit séparateur
    {
      for (i = 20; 13 <= i; i--)
      {
        bite = readBit();
        if (bite != 2)
        {
          Tab[i] = bite;
        }
      }
    }
  }
  else //F21toF28
  {
    bite = readBit();
    if (bite == 0) //bit séparateur
    {
      for (i = 28; 21 <= i; i--)
      {
        bite = readBit();
        if (bite != 2)
        {
          Tab[i] = bite;
        }
      }
    }
  }
}


byte my_pow(int p)

{ if (p == 0)
  {
    return (1);
  }
  return (my_pow(p - 1) * 2);
}



int my_pow_int(int p)
{ if (p == 0)
  {
    return (1);
  }
  return (my_pow(p - 1) * 2);
}

void testTrame()
{
  waitStartTrame();
  int i;
  byte tab[150];
  for (i = 0; i < 150; i++)
  {
    tab[i] = readBit();
  }
  for (i = 0; i < 150; i++)
  {
    Serial.print(tab[i]);
  }
}
