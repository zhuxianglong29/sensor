
#ifndef _SENSOR_H
#define _SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#define ENABLE_DBG  1       //注释掉这句就可省略运行过程，不打印。把1改成0也可以不打印
#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)//？
#endif

#define DFRobot_Sensor_IIC_ADDR 0x66 //芯片IIC地址，无变化地址功能
#define DFRobot_Sensor_ID 0xDF //芯片IIC地址，无变化地址功能

#define SENSOR_ADDR_LED    0 //LED控制地址  这里的描述从芯片手册上抄写
#define SENSOR_ADDR_DATA   2 //组合数据地址  这里的描述从芯片手册上抄写
#define SENSOR_ADDR_CONFIG 3 //配置寄存器地址 这里的描述从芯片手册上抄写
#define SENSOR_ADDR_ID     4 //芯片ID寄存器地址 这里的描述从芯片手册上抄写

#define  COLOR_RGB565_BLACK     0x0000      // 黑色   
#define  COLOR_RGB565_NAVY      0x000F      // 深蓝色  
#define  COLOR_RGB565_DGREEN    0x03E0      // 深绿色  
#define  COLOR_RGB565_DCYAN     0x03EF      // 深青色  
#define  COLOR_RGB565_PURPLE    0x780F      // 紫色  
#define  COLOR_RGB565_MAROON    0x7800      // 深红色      
#define  COLOR_RGB565_OLIVE     0x7BE0      // 橄榄绿      
#define  COLOR_RGB565_LGRAY     0xC618      // 灰白色
#define  COLOR_RGB565_DGRAY     0x7BEF      // 深灰色      
#define  COLOR_RGB565_BLUE      0x001F      // 蓝色   
#define  COLOR_RGB565_GREEN     0x07E0      // 绿色         
#define  COLOR_RGB565_CYAN      0x07FF      // 青色  
#define  COLOR_RGB565_RED       0xF800      // 红色      
#define  COLOR_RGB565_MAGENTA   0xF81F      // 品红   
#define  COLOR_RGB565_YELLOW    0xFFE0      // 黄色        
#define  COLOR_RGB565_WHITE     0xFFFF      // 白色






class DFRobot_Sensor
{
public:

#define ERR_OK             0      ///< 无错误
#define ERR_DATA_BUS      -1      ///< 数据总线错误
#define ERR_IC_VERSION    -2      ///< 芯片版本不匹配


	typedef struct {
		uint8_t   light: 4;
		uint8_t   sound: 4;
	  }__attribute__ ((packed)) sCombinedData_t;
	  
	typedef struct {
		uint8_t   b: 5;
		uint8_t   g: 6;
		uint8_t   r: 5;
	  } __attribute__ ((packed)) sColor_t;
	  
	typedef enum{
    eNormalPower = 0, /**< 正常功耗模式，功耗范围20mW-60mW，可以搭配任意采集速度eSpeedMode_t和采集精度ePrecisionMode_t */
    eLowPower = 1, /**< 低功耗模式，功耗范围2mW-4mW, 注意在低功耗模式下，采集速度eSpeedMode_t只能搭配eNormalSpeed，采集精度ePrecisionMode_t只能搭配eLowPrecision和eNomalPrecision*/
  }eLowPowerMode_t;
  
  /**
   * @enum eSpeedMode_t
   * @brief 简述枚举用途
   * @details 提示一些必要的技术细节(从数据手册上抄写)
   * @note 注解
   * @attention 注意事项
   */
  typedef enum{
    eNormalSpeed = 0<<1, /**< 正常采集速度，可以和任意精度搭配使用 */
    eHighSpeed = 1<<1,   /**< 高速采集模式，采集周期10ms，可以进入低功耗，可以配置为eLowPrecision和eNomalPrecision两种精度模式 */
  }eSpeedMode_t;
  
  /**
   * @enum ePrecisionMode_t
   * @brief 简述枚举用途
   * @details 提示一些必要的技术细节(从数据手册上抄写)
   * @note 注解
   * @attention 注意事项
   */
  typedef enum{
    eLowPrecision   = 0<<2, /**< 低精度，精度大概在xxx，在低精度模式下，可以进入低功耗 */
    eNomalPrecision = 1<<2, /**< 正常精度，精度大概在xxx，在正常精度模式下，可以进入低功耗 */
    eHighPrecision  = 2<<2, /**< 高精度，精度大概在xxx，在高精度模式下，采集速率会降低，采集周期100ms，不能进入低功耗 */
    eUltraPrecision = 3<<2, /**< 超高精度，精度大概在xxx，在超高精度模式下，采集速率会极低，采集周期1000ms，不能进入低功耗 */
  }ePrecisionMode_t;
  DFRobot_Sensor(uint8_t mode);
  virtual int begin(void); //子类的初始化方法不同，父类不会去实现这个函数，加上virtual声明为虚函数，交给子类去实现
  uint16_t soundStrengthDB(void);
  uint32_t lightStrengthLux(void);
  uint8_t switchMode(uint8_t mode);
  void setLED(uint8_t r, uint8_t g, uint8_t b);
  void setLED(uint16_t color);
protected:
  virtual void writeReg(uint8_t reg, void* pBuf, size_t size) = 0;  //父类声明纯虚函数是想让子类实现，来兼容各种接口的芯片，例如IIC、SPI。
  virtual uint8_t readReg(uint8_t reg, void* pBuf, size_t size) = 0;
  private:
	uint8_t _mode;
	TwoWire *_pWire;
	
};



class DFRobot_Sensor_IIC:public DFRobot_Sensor
{
public:
  DFRobot_Sensor_IIC(TwoWire *pWire=&Wire, uint8_t mode=eNomalPrecision+eNormalSpeed+eNormalPower);  //子类的构造函数里加上了默认参数,mode=精度、采样速率、功耗。*pWire=&Wire？
  virtual int begin(void);  //与众不同的功能，在IIC子类中这是初始化IIC//为啥要虚构函数？
protected:
   virtual void writeReg(uint8_t reg, void* pBuf, size_t size);  //这是通过IIC写寄存器
  virtual uint8_t readReg(uint8_t reg, void* pBuf, size_t size);  //这是通过IIC读寄存器
  private:
  uint8_t _deviceAddr;
  TwoWire *_pWire;
};

class DFRobot_Sensor_SPI:public DFRobot_Sensor
{
	public:
  DFRobot_Sensor_SPI(SPIClass *spi=&SPI, uint8_t csPin=4, uint8_t mode=eNomalPrecision+eNormalSpeed+eNormalPower);  //SPI.h里定义了extern SPIClass SPI;因此取SPI对象的地址就能够指向并使用SPI中的方法  //第二个参数csPin是指定cs接的数字引脚
  virtual int begin(void);
protected:
  virtual void writeReg(uint8_t reg, void* pBuf, size_t size) = 0;//为啥写纯虚构函数？
  virtual uint8_t readReg(uint8_t reg, void* pBuf, size_t size) = 0;
  private:
  SPIClass *_pSpi;
  uint8_t _csPin; 
	
};
#endif