

#define ASSERT(cond, msg)					\
if (!(cond)) {							\
printf("assertion '%s' violated in %s line %d (function = %s)", 	\
    msg, __FILE__, __LINE__, __FUNCTION__);			\
exit(-1);							\
}
