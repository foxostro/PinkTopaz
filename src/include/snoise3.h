//
//  snoise3.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/25/12.
//

#ifndef snoise_h
#define snoise_h

#ifdef __cplusplus
extern "C" {
#endif

struct NoiseContext;

float FeepingCreature_noise3(struct NoiseContext *nc, float x, float y, float z);
struct NoiseContext* FeepingCreature_CreateNoiseContext(unsigned *pseed);
void FeepingCreature_DestroyNoiseContext(struct NoiseContext *nc);

#ifdef __cplusplus
}
#endif

#endif /* snoise_h */
