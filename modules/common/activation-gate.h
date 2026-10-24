#include <stdbool.h>
#include <pthread.h>
typedef struct {pthread_mutex_t lock;bool ready,aborted;} LabGate;
#define LAB_GATE_INITIALIZER {PTHREAD_MUTEX_INITIALIZER,false,false}
static bool gate_ready_or_abort(LabGate *gate){pthread_mutex_lock(&gate->lock);bool ready=gate->ready;if(!ready)gate->aborted=true;pthread_mutex_unlock(&gate->lock);return ready;}
