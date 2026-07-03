
// plot settings
#define CYCLE_RANGE 	25
#define STAT_RANGE  	1000
#define RUN_RANGE   	1000
#define RUN_RANGE_HI   	990
#define RUN_RANGE_LO   	10
#define RUN_LENGTH  	1000
#define ZOOM_LENGTH  	1000
#define ZOOM_RANGE  	100
#define ZOOM_SMOOTH 	15
#define ZOOM_GAIN   	8

/* dynamic fields */
typedef struct {
	int32_t * Value;
	int32_t Divisor;
	char * HtmlField;
	int32_t HtmlFieldLength;
	int32_t FractionLength;
	bool isLeftAligned;
	bool isLeadingZero;
	bool isNegative;
	bool isNumeric;

} DynFields_t;

/* send timeflow calibration message */
bool sendTimeflowCalibrationMessage(struct netconn *_conn);

/* send telemetry message */
bool sendTimeflowSolarPowerMessage(struct netconn *_conn);

/* send timeflow ping */
bool sendTimeflowPing(struct netconn *_conn);

/* send html response */
void sendHtmlResponse(struct netconn *_conn);

/* init dynamic web content */
void initDynamicWebContent();

/* parse dynamic fields
 *  ` -> start/end of dynamic content
 *  series of ` -> dynamic text field
 *  0 or + or - or . enclosed in ` -> dynamic numeric field
 *  0 -> positive numeric value with leading zeros
 *  + -> positive numeric value
 *  - -> numeric value may become negative
 *  . -> start of fraction of a float value
 */
void parseDynamicFields(char *_HtmlFile, int _HtmlFileLength, DynFields_t* _DynFields, int _DynFieldsSize);

/* update dynamic fields */
void updateDynamicFields(DynFields_t* _DynFields, int _DynFieldsSize);

/* dynamic integer insertion */
void updateValueInt3(char *_Field, int _ValueInt3);

/* dynamic content insertion */
void updateValue(char *_Field, int _Length, int _Value, int _Point, bool _isLeftAligned, bool _isLeadingZero, bool _isNegative);

/* update measurement curve */
void updateMeasurement(char *_Field, int _FieldLength, int16_t *_Buffer, int _BufferLength, int _ValueMin, int _ValueMax, int _OffsetX);

