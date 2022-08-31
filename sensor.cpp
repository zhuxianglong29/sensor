#include <sensor.h>  






DFRobot_Sensor::DFRobot_Sensor(uint8_t mode)
:_mode(*((uint8_t*)&mode))   //知识点：C++中构造函数才有的初始化列表，可以进行变量的初始化，这里的功能是初始化_mode
{
	
  
}
int DFRobot_Sensor::begin(void)
{
  uint8_t id=0xDF;
  uint8_t buf;
  if(readReg(SENSOR_ADDR_ID,&buf,1) == 0){//#define SENSOR_ADDR_ID     4 //芯片ID寄存器地址 这里的描述从芯片手册上抄写 ，
  //readReg的具体功能见下文
    DBG("bus data access error");
    return ERR_DATA_BUS;  //#define ERR_DATA_BUS      -1      //数据总线错误
  }
  DBG("real sensor id=");DBG(id);
  if(id !=DFRobot_Sensor_ID){
    return ERR_IC_VERSION;  //#define ERR_IC_VERSION    -2      //芯片版本不匹配
  }
  writeReg(SENSOR_ADDR_CONFIG,&_mode,1); //向配置寄存器的地址写入要设置的模式，初始化从机
  DBG("begin ok!");
  return ERR_OK;  //#define ERR_OK             0      //无错误
}
uint16_t DFRobot_Sensor::soundStrengthDB(void)
{
  sCombinedData_t data;  //data这个结构体变量占一个字节，根据手册上这个寄存器定义的，前4位存放声音强度，后4位存放光线强度
  readReg(SENSOR_ADDR_DATA, &data, sizeof(data));  //从SENSOR_ADDR_DATA（组合数据地址）读数据，存放到data中，所以传的是data的地址
  DBG("sound reg raw data is");
  DBG(data.sound);  
  return data.sound << 3; //单位是DB，所以要移位？？
}
uint32_t DFRobot_Sensor::lightStrengthLux(void)
{
  sCombinedData_t data;
  readReg(SENSOR_ADDR_DATA, &data, sizeof(data));
  DBG("light reg raw data is");
  DBG(data.light);
  return data.light * 10000;
}
uint8_t DFRobot_Sensor::switchMode(uint8_t mode)
{
  uint8_t tmp;
  #ifdef ENABLE_DBG
  readReg(SENSOR_ADDR_CONFIG, &tmp, sizeof(tmp));
  DBG("before switch Mode, ModeReg = ");
  DBG(mode);
  #endif
  writeReg(SENSOR_ADDR_CONFIG, &mode, sizeof(mode));
  #ifdef ENABLE_DBG
  readReg(SENSOR_ADDR_CONFIG, &tmp, sizeof(tmp));
  DBG("after switch Mode, ModeReg = ");
  DBG(tmp);
  #endif
  /*
    return
  */
}
void DFRobot_Sensor::setLED(uint8_t r, uint8_t g, uint8_t b)
{
  sColor_t data={.b=b>>3,.g=g>>2,.r=r>>3};
  writeReg(SENSOR_ADDR_LED, &data, sizeof(data));
}

void DFRobot_Sensor::setLED(uint16_t color)
{
  writeReg(SENSOR_ADDR_LED, &color, sizeof(color));
}


DFRobot_Sensor_IIC::DFRobot_Sensor_IIC(TwoWire *pWire, uint8_t mode)
  :DFRobot_Sensor(mode)  //初始化列表：子类用父类给的构造函数（就是上一个框里的函数）从而初始化了父类的私有变量_mode
{
  _deviceAddr = DFRobot_Sensor_IIC_ADDR;        //因为用到了宏定义，所以单独注释说明，源码的.cpp是没有这个注释的
  _pWire = pWire;     //子类构造函数给自己的成员赋值，什么意思呢，下面是涉及的宏定义
}
//在宏定义里有芯片IIC地址：  #define DFRobot_Sensor_IIC_ADDR 0x66
//DFRobot_Sensor_IIC类的private中定义和表示Wire对象的地址的变量：  TwoWire *_pWire;uint8_t _deviceAddr;



int DFRobot_Sensor_IIC::begin(void)
{
  Wire.begin();   //Wire.h（IIC）库函数 初始化wire库
  return DFRobot_Sensor::begin();  //执行上面的初始化函数
}



void DFRobot_Sensor_IIC::writeReg(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
          DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_deviceAddr);//芯片地址DFRobot_Sensor_I2C_ADDR 0x66
 _pWire->write(&reg, 1);

  for(uint16_t i = 0; i < size; i++){
     _pWire->write(_pBuf[i]);  //将以地址方式传进来的mode内容通过Wire.write函数写给从机寄存器，完成初始化
  }
  _pWire->endTransmission();
}

uint8_t DFRobot_Sensor_IIC::readReg(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf =  (uint8_t*)pBuf; //传进来的是buf的地址，通过指针修改buf的内容//_pBuf定义在wire里的？
  _pWire->beginTransmission(_deviceAddr);
   _pWire->write(& reg, 1);                         //用到了Wire.write()
  if( _pWire->endTransmission() != 0){                 //用到了 Wire.endTransmission()，结束一个由beginTransmission（）开始的并且由write（）排列的从机的传输。返回0 成功 1 数据溢出 2 发送错误或其他错误
      Serial.println("Wire.endTransmission error");
      return 0;
  }
  _pWire->requestFrom(_deviceAddr, (uint8_t) size);        //主设备请求从设备一个字节，这个字节可以被主设备用 read()或available()接受
  for(uint16_t i = 0; i < size; i++){
    _pBuf[i] = _pWire->read();        //用read()接收放到buf里
  }
  _pWire->endTransmission();
  return size;
}

//SPI部分待完善
int DFRobot_Sensor_SPI::begin(void)
{
  pinMode(_csPin, OUTPUT);
  _pSpi->begin();
  return DFRobot_Sensor::begin();
}

void DFRobot_Sensor_SPI::writeReg(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
	  DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pSpi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin, 0);
  _pSpi->transfer(reg);
  while(size--) {
    _pSpi->transfer(*_pBuf);
    _pBuf ++;
  }
  digitalWrite(_csPin, 1);
  SPI.endTransaction();
}

uint8_t DFRobot_Sensor_SPI::readReg(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
	  DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  size_t count = 0;
  _pSpi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin, 0);
  _pSpi->transfer(reg);
  while(size--) {
    *_pBuf = SPI.transfer(0x00);
    _pBuf++;
    count++;
  }
  digitalWrite(_csPin, 1);
  _pSpi->endTransaction();
  return count;
}



