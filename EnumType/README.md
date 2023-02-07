# EnumType Library

此库用于自动生成枚举及函数`GetString ` `GetEnumTypeValue`

**定义：**

````c
#define TEMP_ENUM(XX) \
	XX(IDLE,) \
	XX(STATUS1,) \
	XX(STATUS2,) \
	XX(STATUS50,=50) \
	
ENUM_DEFINE(TempEnum,TEMP_ENUM)
````



**使用：**

````c
void test(void)
{
    char *str;
    int enum_value;
    
    str = (char *)GetString(STATUS50);
    enum_value = GetTempEnumValue("STATUS50");
    
    printf("str is %s\n",str);
    printf("enum_value is %d\n",enum_value);
}
````























