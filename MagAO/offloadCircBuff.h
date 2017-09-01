#ifndef __offloadCircBuff_h__
#define __offloadCircBuff_h__



struct circBuffEntry
{
   double timeStamp;
   std::vector<float> entry;
};

class offloadCircBuff
{

public:
   int _buffLen;
   int _offloadLen;


   int _currEntry;
   int _nEntries;


   std::vector<circBuffEntry> entries;

   offloadCircBuff()
   {
      _buffLen = 0;
      _offloadLen = 0;

      _currEntry = 0;
      _nEntries = 0;
   }      



   void setLength(int bf, int oll)
   {
      _buffLen = bf;
      _offloadLen = oll;

      _currEntry = 0;
      _nEntries = 0;


      entries.resize(_buffLen);

      for(int i=0;i<_buffLen; ++i)
      {
         entries[i].timeStamp = 0;
         entries[i].entry.resize(_offloadLen,0);
      }
   }


   void addEntry(double ts, float * z)
   {
      
      if(_currEntry+1 >= _buffLen) _currEntry = 0;
      else ++_currEntry;

      entries[_currEntry].timeStamp = ts;

      for(int i=0;i<_offloadLen; ++i) 
      {
         entries[_currEntry].entry[i] = z[i];
      }
      
      if(_nEntries < _buffLen) ++_nEntries;
   }

   float average(int no, double len)
   {

      if(len == 0) return entries[_currEntry].entry[no];
      
      int idx = _currEntry;
      double t0 = entries[idx].timeStamp;

      float avg = 0;
      int n = 0;
      for(int i =0; i< _nEntries; ++i)
      {
         avg += entries[idx].entry[no];
         ++n;
         --idx;
         if(idx <= 0) idx = _buffLen - 1;

         if( t0 - entries[idx].timeStamp > len) break;
      }

      return avg/n;
   }

};

            


#endif //__offloadCircBuff_h__

      




