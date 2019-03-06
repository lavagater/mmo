
//passed to shared pointer when the shared pointer is being used for an array
template< typename T >
struct array_deleter
{
  void operator ()( T const * p)
  { 
    delete[] p; 
  }
};