#ifndef vgp_error_h
#define vgp_error_h

#define __ensure_m3_result(res,retn) if (res) { __vgp_set_last_error(res); return retn; }

const char *vgp_get_last_error(void);
void __vgp_set_last_error(const char *error);

#endif // vgp_error_h
