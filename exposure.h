/**
 * $Revision$
 * $Date$
 * $Author$
 */

#ifndef EXPOSURE_H_
#define EXPOSURE_H_

typedef   signed char ec_t;
typedef unsigned char ev_t;
typedef unsigned char av_t;
typedef unsigned char tv_t;
typedef unsigned char iso_t;

#define EV_CODE(val, sub)  (0010 * (val) + (sub))

#define EV_VAL(code)  ((code) / 0010)
#define EV_SUB(code)  ((code) - 0010 * EV_VAL(code))

#define EC_MIN EV_CODE(+6, 0)  // +6EV
#define EC_MAX EV_CODE(-6, 0)  // -6EV

#define TV_MIN EV_CODE( 2, 0)  // 30"
#define TV_MAX EV_CODE(23, 0)  // 1/4000s

#define BULB_MIN 0040  // 30'
#define BULB_MAX 0330  // 1/4000s

#define ISO_MIN EV_CODE( 9, 0)
#define ISO_MAX EV_CODE(13, 0)
#define ISO_EXT EV_CODE(13, 7)

extern ec_t ec_normalize(ec_t ec);

extern ec_t ec_inc(ec_t ev);
extern ec_t ec_dec(ec_t ev);
extern ec_t ec_add(ec_t ying, ec_t yang);
extern ec_t ec_sub(ec_t ying, ec_t yang);

extern void ec_print(char *dest, ec_t ec);

extern av_t av_add(av_t ying, av_t yang);
extern av_t av_sub(av_t ying, av_t yang);
extern av_t av_inc(av_t av);
extern av_t av_dec(av_t av);

extern void av_print(char *dest, av_t av);

extern tv_t tv_add(tv_t ying, tv_t yang);
extern tv_t tv_sub(tv_t ying, tv_t yang);
extern tv_t tv_inc(tv_t tv);
extern tv_t tv_dec(tv_t tv);

extern tv_t bulb_next(tv_t tv);
extern tv_t bulb_prev(tv_t tv);

extern void tv_print  (char *dest, tv_t tv);
extern void bulb_print(char *dest, tv_t tv);

extern iso_t iso_roll(iso_t iso);
extern iso_t iso_next(iso_t iso);
extern iso_t iso_prev(iso_t iso);
extern iso_t iso_inc(iso_t iso);
extern iso_t iso_dec(iso_t iso);

extern void iso_print(char *dest, iso_t code);

#endif /* EXPOSURE_H_ */
