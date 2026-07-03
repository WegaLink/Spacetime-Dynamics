
/* verify delay line */
void verifyDelayLine();

/* find optimal delay line setup */
void setupDelayLine();

/* set delay line step 0..255 */
void setDelayStep(int _DelayStep);

/* verify response curve */
void updateResponse(int16_t *_ResponsePlot, int _ResponseLength, int _ResponseRange);

/* initialize pulse acquisition */
void initPulse(void);

