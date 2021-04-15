#include <locale>
#include <cmath>
#include <vector>
#include <sstream>
#include <iomanip>

class fixed final
{
private:
    static const int fractional_bits = 16;
    static const int fixed_type_bits = 32;
    static const int fractional_mask = 0xffff;
    
    typedef uint32_t fixed_type;
    typedef uint64_t expand_type;
    
    fixed_type value;

    uint32_t _pow(uint32_t x, uint32_t p)
    {
      if (p == 0) return 1;
      if (p == 1) return x;
 
      uint32_t tmp = _pow(x, p/2);
      if (p%2 == 0) return tmp * tmp;
      else return x * tmp * tmp;
    }

    fixed_type fixed_mult(fixed_type inp_1, fixed_type inp_2) const
    {
        bool sign = ((inp_1&(1<<31)) - (inp_2&(1<<31))) != 0;
        fixed_type t_inp_1 = (inp_1&(1<<31)) ? -inp_1 : inp_1;
        fixed_type t_inp_2 = (inp_2&(1<<31)) ? -inp_2 : inp_2;

        fixed_type result = (fixed_type)(((expand_type)t_inp_1 * (expand_type)t_inp_2) >> fractional_bits);
        if(sign) result = -result;
        return result;
    }

    fixed_type fixed_div(fixed_type inp_1, fixed_type inp_2) const
    {
        bool sign = ((inp_1&(1<<31)) - (inp_2&(1<<31))) != 0;
        fixed_type t_inp_1 = (inp_1&(1<<31)) ? -inp_1 : inp_1;
        fixed_type t_inp_2 = (inp_2&(1<<31)) ? -inp_2 : inp_2;

        fixed_type result = (fixed_type)(((expand_type)t_inp_1 * (1 << fractional_bits))/ (expand_type)t_inp_2);
        if(sign) result = -result;
        return result;
    }

    fixed_type fixed_add(fixed_type inp_1, fixed_type inp_2) const
    {
        fixed_type inp_1_sign = inp_1 >> (fixed_type_bits - 1);
        fixed_type inp_2_sign = inp_2 >> (fixed_type_bits - 1);
        fixed_type add = inp_1 + inp_2;
        fixed_type add_sign = add >> (fixed_type_bits - 1);

        if (inp_1_sign != inp_2_sign)
        {
            return add;
        }
        else if (add_sign == inp_1_sign)
        {
            return add;
        }
        else if (add_sign == -1)
        {
            return ((1 << (fixed_type_bits - 2)) - 1 + (1 << (fixed_type_bits - 2)));
        }
        else if (add_sign == 1)
        {
            return (1 << (fixed_type_bits - 1));
        }
        return 0;
    }
    fixed_type fixed_sub(fixed_type inp_1, fixed_type inp_2) const
    {
        fixed_type inp_1_sign = inp_1 >> (fixed_type_bits - 1);
        fixed_type inp_2_sign = inp_2 >> (fixed_type_bits - 1);
        fixed_type sub = inp_1 - inp_2;
        fixed_type sub_sign = sub << (fixed_type_bits - 1);

        if (inp_1_sign != inp_2_sign)
        {
            return sub;
        }
        else if (sub_sign == inp_1_sign)
        {
            return sub;
        }
        else if (sub_sign == -1)
        {
            return ((1 << (fixed_type_bits - 2)) - 1 + (1 << (fixed_type_bits - 2)));
        }
        else if (sub_sign == 1)
        {
            return (1 << (fixed_type_bits - 1));
        }
        return 0;
    }
    fixed_type fromFloat(float value)
    {
      fixed_type result = *(fixed_type*)&value;
      // INFINITY
      if((result&0x7fffffff) == 0x7f800000)
        throw std::string("INFINITY");
      // NaN
      if((result&0x7fffffff) > 0x7f800000)
        throw std::string("NaN");
      // Null
      if((result&0x7fffffff) == 0)
        return 0;
      // Denormalized
      if((result&0x7fffffff) < 0x00800000)
        throw std::string("Denormalized");
      
      bool sign = (result & 0x80000000);
      uint_fast8_t exponent = (result >> 23) & 0xff;

      if(exponent > 141 || exponent < 110)
        throw std::string("Out of range");

      result = (result&0x007fffff)|(1<<23);
      int8_t shift = (127 + fractional_bits - 9 - exponent);

      if(shift > 0)
        result >>= shift;
      else
        result <<= -shift;

      if(sign)
      {
        //result = -result;
          int32_t temp = ((result & 0xffff0000) >> 16);
          result = ((~--temp) << 16) + (result & 0x0000ffff);
      }
      std::cout << std::hex << result << std::endl;
      return result; 
    }
public:
    fixed() {}
    fixed(float value)
    {
      this->value = fromFloat(value);
    }
    fixed(double value)
    {
      *this = (float)value;
    }
    fixed(int value)
    {
      if(value > 32767)
        throw std::string("Out of range");

      this->value = (fixed_type)((expand_type)value * (1 << fractional_bits));
    }
    static fixed fromRaw(fixed_type value)
    {
      fixed result;
      result.value = value;
      return result;
    }
    int16_t toInt() const
    {
      return (value >> fractional_bits);
    }
    int32_t toRaw() const
    {
      return value;
    }
    float toFloat() const
    {
      fixed_type _value = value;
      uint_fast8_t exponent = 134;

      bool sign = (_value & 0x80000000);
      if(sign)
      {
          //_value = -_value;
          int32_t temp = ((_value & 0xffff0000) >> 16);
          _value = ((~--temp) << 16) + (_value & 0x0000ffff);
      }

      if(_value != 0)
      {
          if(_value & 0xff000000)
          {
              while((_value & 0xff000000))
              {
                  _value >>= 1;
                  exponent ++;
              }
          }
          else
          {
              while((_value & 0xff800000) == 0)
              {
                  _value <<= 1;
                  exponent --;
              }
          }
          _value &= 0x007fffff;
          _value |= (uint32_t)exponent << 23;
          if(sign) _value |= 0x80000000;
      }
      return *(float*)&_value;      
    }
    static fixed fromString(const std::string& stringValue)
    {
      std::vector<std::string> token_v;
      fixed result;
      char decimal_point = std::use_facet< std::numpunct<char> >(std::cout.getloc()).decimal_point();
      size_t start = stringValue.find_first_not_of(decimal_point), end=start;

      while (start != std::string::npos){
        end = stringValue.find(decimal_point, start);
        token_v.push_back(stringValue.substr(start, end-start));
        start = stringValue.find_first_not_of(decimal_point, end);
      }

      if(token_v.size() > 2 || token_v.empty())
      {
        throw std::string("Invalid argument");
      }

      try
      {
        result.value = (std::stoi(token_v.at(0)) << fractional_bits);
        if(token_v.size() > 1)
        {
          uint32_t fr_size = result._pow(10, token_v.at(1).size());
          result.value |= ((std::stoi(token_v.at(1)) << fractional_bits) / fr_size);
        }
      }
      catch (...)
      {
        throw std::string("Invalid argument");
      }
      return result;
    }
    std::string toString(int precision = 2)
    {
      std::cout << std::hex << value << std::endl;
      std::stringstream result;
      if(!(precision &= 7)) precision = 2;
      int16_t integer = value >> fractional_bits;
      uint64_t fr_size = _pow(10, precision + 1);
      uint64_t fraction = (((value & fractional_mask) * fr_size) >> fractional_bits);
      if((fraction % 10) >= 5)
      {
        fraction += 10;
      }
      fraction /= 10;
      result << integer;
      if(fraction)
      {
        char decimal_point = std::use_facet< std::numpunct<char> >(std::cout.getloc()).decimal_point();
        result  << decimal_point << std::setw(precision) << std::setfill('0') << fraction;
      }
      return result.str();
    }

    //
    // бинарные
    friend fixed& operator+=(fixed& lv, const fixed& rv);
    friend fixed& operator-=(fixed& lv, const fixed& rv);
    friend fixed& operator*=(fixed& lv, const fixed& rv);
    friend fixed& operator/=(fixed& lv, const fixed& rv);
    friend const fixed operator-(const fixed& lv, const fixed& rv);
    friend const fixed operator+(const fixed& lv, const fixed& rv);
    friend const fixed operator-(const fixed& lv, int rv);
    friend const fixed operator+(const fixed& lv, int rv);
    friend const fixed operator-(const fixed& lv, float rv);
    friend const fixed operator+(const fixed& lv, float rv);
    friend const fixed operator-(const fixed& lv, double rv);
    friend const fixed operator+(const fixed& lv, double rv);
    friend const fixed operator-(int lv, const fixed& rv);
    friend const fixed operator+(int lv, const fixed& rv);
    friend const fixed operator-(float lv, const fixed& rv);
    friend const fixed operator+(float lv, const fixed& rv);
    friend const fixed operator-(double lv, const fixed& rv);
    friend const fixed operator+(double lv, const fixed& rv);
    friend const fixed operator*(int lv, const fixed& rv);
    friend const fixed operator/(int lv, const fixed& rv);
    friend const fixed operator*(float lv, const fixed& rv);
    friend const fixed operator/(float lv, const fixed& rv);
    friend const fixed operator*(double lv, const fixed& rv);
    friend const fixed operator/(double lv, const fixed& rv);
    friend const fixed operator*(const fixed& lv, const fixed& rv);
    friend const fixed operator/(const fixed& lv, const fixed& rv);
    friend const fixed operator*(const fixed& lv, int rv);
    friend const fixed operator/(const fixed& lv, int rv);
    friend const fixed operator*(const fixed& lv, float rv);
    friend const fixed operator/(const fixed& lv, float rv);
    friend const fixed operator*(const fixed& lv, double rv);
    friend const fixed operator/(const fixed& lv, double rv);
    // операторы сравнения
    friend bool operator==(const fixed& lv, const fixed& rv);
    friend bool operator!=(const fixed& lv, const fixed& rv);
    friend bool operator>(const fixed& lv, const fixed& rv);
    friend bool operator<(const fixed& lv, const fixed& rv);
    friend bool operator>=(const fixed& lv, const fixed& rv);
    friend bool operator<=(const fixed& lv, const fixed& rv);
    // перегрузка преобразования типа
    operator float()/* const*/
    {
      return float(toFloat());
    }
    explicit operator int() const
    {
      return float(toInt());
    }
};

const fixed operator-(const fixed& lv, const fixed& rv) {
   return fixed::fromRaw(lv.fixed_sub(lv.value, rv.value));
}
const fixed operator+(const fixed& lv, const fixed& rv) {
   return fixed::fromRaw(lv.fixed_add(lv.value, rv.value));
}
const fixed operator-(const fixed& lv, int rv) {
   return lv - fixed(rv);
}
const fixed operator+(const fixed& lv, int rv) {
   return lv + fixed(rv);
}
const fixed operator-(const fixed& lv, float rv) {
   return lv - fixed(rv);
}
const fixed operator+(const fixed& lv, float rv) {
   return lv + fixed(rv);
}
const fixed operator-(const fixed& lv, double& rv) {
   return lv - fixed(rv);
}
const fixed operator+(const fixed& lv, double rv) {
   return lv + fixed(rv);
}

const fixed operator*(const fixed& lv, const fixed& rv)
{
   return fixed::fromRaw(lv.fixed_mult(lv.value, rv.value));
}
const fixed operator/(const fixed& lv, const fixed& rv)
{
   return fixed::fromRaw(lv.fixed_div(lv.value, rv.value));
}
const fixed operator*(const fixed& lv, int rv)
{
   return lv * fixed(rv);
}
const fixed operator/(const fixed& lv, int rv)
{
   return lv / fixed(rv);
}
const fixed operator*(const fixed& lv, float rv)
{
   return lv * fixed(rv);
}
const fixed operator/(const fixed& lv, float rv)
{
   return lv / fixed(rv);
}
const fixed operator*(const fixed& lv, double rv)
{
   return lv * fixed(rv);
}
const fixed operator/(const fixed& lv, double rv)
{
   return lv / fixed(rv);
}
const fixed operator-(int lv, const fixed& rv)
{
  return fixed(lv) - rv;
}
const fixed operator+(int lv, const fixed& rv)
{
  return fixed(lv) + rv;
}
const fixed operator-(float lv, const fixed& rv)
{
  return fixed(lv) - rv;
}
const fixed operator+(float lv, const fixed& rv)
{
  return fixed(lv) + rv;
}
const fixed operator-(double lv, const fixed& rv)
{
  return fixed(lv) - rv;
}
const fixed operator+(double lv, const fixed& rv)
{
  return fixed(lv) + rv;
}
const fixed operator*(int lv, const fixed& rv)
{
  return fixed(lv) * rv;
}
const fixed operator/(int lv, const fixed& rv)
{
  return fixed(lv) / rv;
}
const fixed operator*(float lv, const fixed& rv)
{
  return fixed(lv) * rv;
}
const fixed operator/(float lv, const fixed& rv)
{
  return fixed(lv) / rv;
}
const fixed operator*(double lv, const fixed& rv)
{
  return fixed(lv) * rv;
}
const fixed operator/(double lv, const fixed& rv)
{
  return fixed(lv) / rv;
}
fixed& operator+=(fixed& lv, const fixed& rv) {
    lv = lv + rv;
    return lv;
}
fixed& operator-=(fixed& lv, const fixed& rv) {
    lv = lv - rv;
    return lv;
}
fixed& operator*=(fixed& lv, const fixed& rv) {
    lv = lv * rv;
    return lv;
}
fixed& operator/=(fixed& lv, const fixed& rv) {
    lv = lv / rv;
    return lv;
}
bool operator==(const fixed& lv, const fixed& rv)
{
  return lv.value == rv.value;
}
bool operator!=(const fixed& lv, const fixed& rv)
{
  return lv.value != lv.value;
}
bool operator>(const fixed& lv, const fixed& rv)
{
  return (int32_t)lv.value > (int32_t)rv.value;
}
bool operator<(const fixed& lv, const fixed& rv)
{
  return (int32_t)lv.value < (int32_t)rv.value;
}
bool operator>=(const fixed& lv, const fixed& rv)
{
  return (int32_t)lv.value >= (int32_t)rv.value;
}
bool operator<=(const fixed& lv, const fixed& rv)
{
  return (int32_t)lv.value <= (int32_t)rv.value;
}