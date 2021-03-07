## IRLib2

Laajennus Adafruit Circuit Playground Express IRLib2 -kirjastoon infrapunakoodien lähettämiseen Hästens-sängylle

Hästens-protokolla on numero P13

IRLIb2-kirjasto tulee Adafruilt Circuit Playground Express -kirjaston mukana ja asentuu osoiteeseen:

``/Users/ristohelkio/Documents/Arduino/libraries/Adafruit_Circuit_Playground/utility``


### IRLib_P13_Hastens.h

Lähetys- ja vastaanottoluokat :

```
  class IRsendHastens: public virtual IRsendBase  
  class IRdecodeHastens: public virtual IRdecodeBase  
```

### IRLibProtocols.h

Hästens-protokollan numero:

```
#define HASTENS 13
#define LAST_PROTOCOL 13 //Be sure to update this when adding protocols
```


## Testauksen avuksi

### IRLibSendBase.cpp

Lisätty triggaussignaaleja testauksen helpottamiseksi

### IRLibDecodeBase.cpp 

Lisätty metodi 

```
  void IRdecodeBase::dumpResultsRH(bool verbose) 
```

Metodi tulostaa havaitut infrapunakoodit.