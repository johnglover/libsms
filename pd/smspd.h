#ifndef smspd_h
#define smspd_h

#include "m_pd.h"

void smsbuf_setup(void);
void smsanal_setup(void);
void smssynth_tilde_setup(void);
//void smsedit_setup(void);

//method for opening file in canvas directory.
t_symbol* getFullPathName( t_symbol *infilename,  t_canvas *smsCanvas); 



#endif// smspd_h