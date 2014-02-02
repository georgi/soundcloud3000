#include <stdio.h>
#include <strings.h>
#include <mpg123.h>
#include "soundcloud3000.h"

mpg123_handle *mpg_open_file(const char *filename) {
  int err = MPG123_OK;
  mpg123_handle *mh;
  long rate;
  int channels, encoding;

  if ((mh = mpg123_new(NULL, &err)) == NULL) {
    fprintf(stderr, "%s", mpg123_plain_strerror(err));
    return NULL;
  }

  mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.);

  if (mpg123_open(mh, filename) != MPG123_OK ||
      mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
    fprintf(stderr, "%s", mpg123_strerror(mh));
    return NULL;
  }

  if (encoding != MPG123_ENC_FLOAT_32) {
    fprintf(stderr, "bad encoding");
    return NULL;
  }

  mpg123_format_none(mh);
  mpg123_format(mh, rate, channels, encoding);

  return mh;
}
