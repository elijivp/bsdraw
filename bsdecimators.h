#ifndef BSDECIMATORS_H
#define BSDECIMATORS_H

/// This file contains set of decimators: special procedures who reduce count of input data
/// Created By: Elijah Vlasov

template <class T>
void  decimate_bywindow(const float* from, unsigned int fromsize, T& decimator, float* to, unsigned int tosize, unsigned int portionsCount)
{
  unsigned int stepsize = fromsize / tosize;
  unsigned int laststepsize = fromsize % tosize;
  for (unsigned int p=0; p<portionsCount; p++)
    for (int i=0; i<tosize; i++)
      to[p*tosize + i] = decimator(&from[p*fromsize + stepsize*i], i < tosize - 1? stepsize : laststepsize);
}

inline float window_max(const float* from, unsigned int count){ float result=from[0]; for (unsigned int i=1; i<count; i++)  if (result < from[i]) result = from[i]; return result; }
inline float window_min(const float* from, unsigned int count){ float result=from[0]; for (unsigned int i=1; i<count; i++)  if (result > from[i]) result = from[i]; return result; }
inline float window_mean(const float* from, unsigned int count){ float result=0; for (unsigned int i=0; i<count; i++)  result += from[i]; return result/count; }

inline float window_ez_first(const float* from, unsigned int){ return from[0]; }
inline float window_ez_mid(const float* from, unsigned int count){ return from[count/2]; }
inline float window_ez_last(const float* from, unsigned int count){ return from[count-1]; }

inline float window_mean_nopeaks(const float* from, unsigned int count){  float min=from[0], max=from[0], sum=from[0]; 
                                                                          for (unsigned int i=1; i<count; i++){  
                                                                            if (min > from[i]) min = from[i];
                                                                            if (max < from[i]) max = from[i];
                                                                            sum += from[i];
                                                                          }
                                                                          return (sum - min - max) / count;
                                                                       }


//template <int N>
//class Window_meanmax
//{
//  float accum[N];
//public:
////  Window_meanmax(): inited(false){}
//  float operator()(const float* from, unsigned int count)
//  {
//    for (unsigned int i=0; i<count; i++)
//    {
//      if (i < N)
//      {
//        for (unsigned int j=0; j<N; j++)
//          if (accum[i])
//        accum[i] = from[i];
//      }
//      else
//      {
//        int idx = 0;
//        for (unsigned int j=1; j<N; j++)
//          if (accum[j] < accum[idx])
//      }
//    }
    
//    return window_mean(accum, N);
//  }
//};

#endif // BSDECIMATORS_H
