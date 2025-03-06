#ifndef _TRANSACTIONS_H_
#define _TRANSACTIONS_H_

#include "file_ops.h"

/* Maximum size of a valid transaction line */
#define MAXLINE 1088

/* Parse transfile and feed its transactions to fs */
void process_transactions(char *transfile);

#endif /* _TRANSACTIONS_H_ */
