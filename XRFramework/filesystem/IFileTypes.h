#pragma once

struct SNativeIoControl
{
  unsigned long int   request;
  void*               param;
};

struct SCacheStatus
{
  uint64_t forward;  /**< number of bytes cached forward of current position */
  unsigned maxrate;  /**< maximum number of bytes per second cache is allowed to fill */
  unsigned currate;  /**< average read rate from source file since last position change */
  bool     full;     /**< is the cache full */
};

typedef enum {
  IOCTRL_NATIVE        = 1, /**< SNativeIoControl structure, containing what should be passed to native ioctrl */
  IOCTRL_SEEK_POSSIBLE = 2, /**< return 0 if known not to work, 1 if it should work */
  IOCTRL_CACHE_STATUS  = 3, /**< SCacheStatus structure */
  IOCTRL_CACHE_SETRATE = 4, /**< unsigned int with speed limit for caching in bytes per second */
  IOCTRL_SET_CACHE    = 8, /** <CFileCache */
} EIoControl;
