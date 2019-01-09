#include <clp/libclp.h>
#include <clp/IntSpanArray.h>

//////////////////////////////////////////////////////////////////////////////

UTL_NS_USE;
LUT_NS_USE;

//////////////////////////////////////////////////////////////////////////////

UTL_CLASS_IMPL(clp::IntSpanArray, utl::Object);

//////////////////////////////////////////////////////////////////////////////

CLP_NS_BEGIN;

//////////////////////////////////////////////////////////////////////////////

void
IntSpanArray::add(int min, int max)
{
   if (_size == _reservedSize)
   {
      grow();
   }

   ASSERTD(min <= max);
   uint_t* ptr = _array + (_size << 1);
   *ptr = min;
   *++ptr = max;
   ++_size;
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpanArray::deInit()
{
   delete [] _array;
}

//////////////////////////////////////////////////////////////////////////////

void
IntSpanArray::grow()
{
   ASSERTD(_size == _reservedSize);

   // figure out new size
   uint_t newSize;
   if (_size == 0)
   {
      newSize = 4096;
   }
   else
   {
      newSize = _size << 1;
   }

   uint_t* newArray = new uint_t[newSize << 1];
   uint_t i;
   for (i = 0; i < (_size << 1); ++i)
   {
      newArray[i] = _array[i];
   }

   delete [] _array;
   _array = newArray;
   _reservedSize = newSize;
}

//////////////////////////////////////////////////////////////////////////////

CLP_NS_END;
