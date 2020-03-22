
#include <EEPROM.h>
class DataEEPROM{
    private:
        int direction_data;
    public:
        DataEEPROM(int direction_data2);
        byte readData();
        void writeData(byte data);
        void addOne();
        void resetData();
};
DataEEPROM::DataEEPROM(int direction_data2){
    direction_data=direction_data2; 
}
void DataEEPROM::writeData(byte data){
    EEPROM.write(direction_data, data);
}
byte DataEEPROM::readData(){
    return (byte)EEPROM.read(direction_data);
}
void DataEEPROM::addOne(){
    EEPROM.write(direction_data,EEPROM.read(direction_data)+1);
}
void DataEEPROM::resetData(){
    EEPROM.write(direction_data, 0);
}

class VolumenTidalEEPROM {
    private:
        int direccion_inicial;
        int direccion_final;
    public:
        VolumenTidalEEPROM(int direccion_inicial2);
        int readVolumenTidal();
        boolean isVolumenTidalWritten();
        void writeVolumenTidal(int volumen_tidal);
};

VolumenTidalEEPROM::VolumenTidalEEPROM(int direccion_inicial2){
    direccion_inicial=direccion_inicial2;
    direccion_final=direccion_inicial2+4;
}
int VolumenTidalEEPROM::readVolumenTidal(){
    int finalVolumenTidal=0;
    if(EEPROM.read(direccion_inicial)){
        for(int i=direccion_inicial+1; i<=direccion_final; i++){
            finalVolumenTidal+=(byte)EEPROM.read(i);
        }
        return finalVolumenTidal;
    }else{
        return 0; //false
    }
    
}

void VolumenTidalEEPROM::writeVolumenTidal(int volumen_tidal){
    int blocksToWrite=0;

    EEPROM.write(direccion_inicial, true); 

    if(volumen_tidal%255==0){
        blocksToWrite= ( (int) volumen_tidal/255 ) ;
    }else{
        blocksToWrite= ( (int) volumen_tidal/255 ) +1;
    }
    for(int i=1; i<blocksToWrite; i++){
        if(i==blocksToWrite){
            EEPROM.write(direccion_inicial+blocksToWrite, volumen_tidal);
        }else{
            EEPROM.write(direccion_inicial+i, 255);
            volumen_tidal-=255;
        }
        
    }
}

boolean VolumenTidalEEPROM::isVolumenTidalWritten(){
    boolean is=false;
    if(EEPROM.read(direccion_inicial)==1){
        is=true;
    }
    return is;
}