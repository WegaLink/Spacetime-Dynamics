#include "DAVE.h"
#include "stdio.h"

#include "Html.h"
#include "IoTapp.h"

/* dynamic timeflow calibration message */
DynFields_t DynTimeflowCalibration[] = {
		{&DeviceID_, 1},
		{&PingID_, 1},
		{&Temperature_, 1},
		{&Ucc_, 1000},
		{&Phase_deg_, 1},
		{&CalHighValue_[0], 1000},
		{&CalHighValue_[1], 1000},
		{&CalHighValue_[2], 1000},
		{&CalHighValue_[3], 1000},
		{&CalHighValue_[4], 1000},
		{&CalHighValue_[5], 1000},
		{&CalHighValue_[6], 1000},
		{&CalHighValue_[7], 1000},
		{&CalHighValue_[8], 1000},
		{&CalHighValue_[9], 1000},
		{&CalHighValue_[10], 1000},
		{&CalHighValue_[11], 1000},
		{&CalHighValue_[12], 1000},
		{&CalHighValue_[13], 1000},
		{&CalHighValue_[14], 1000},
		{&CalHighValue_[15], 1000},
		{&CalHighValue_[16], 1000},
		{&CalHighValue_[17], 1000},
		{&CalHighValue_[18], 1000},
		{&CalHighValue_[19], 1000},
		{&CalHighValue_[20], 1000},
};
static int DynTimeflowCalibrationSize = sizeof(DynTimeflowCalibration) / sizeof(DynFields_t);
static char TimeflowCalibration[] = {DATA_OP".FTLightPing#121,"OPERATOR".TimeflowCalibration"SUBJECT_TEST"#`0`,`00000000`,`-`.`°C,`.```V,```.`°,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,+`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`,`+`\n"};

/* dynamic timeflow system power message */
DynFields_t DynTimeflowSolarPower[] = {
		{&DeviceID_, 1},
		{&PingID_, 1},
		{&Temperature_, 1},
		{&Solar_, 10000},
		{&Battery_, 10000},
		{&Power_, 10000},
		{&PoE_, 10000},
};
static int DynTimeflowSolarPowerSize = sizeof(DynTimeflowSolarPower) / sizeof(DynFields_t);
static char TimeflowSolarPower[] = {DATA_OP".FTLightPing#121,"OPERATOR".TimeflowSolarPower"SUBJECT_TEST"#`0`,`00000000`,`-`.`°C,sol:`-`.``V,bat:`-`.``V,pow:``.``W,poe:``.``V\n"};

/* dynamic timeflow ping  */
DynFields_t DynTimeflowPing[] = {
		{&DeviceID_, 1},
		{&PingID_, 1},
		{&Temperature_, 1},
		{&Ucc_, 1000},
		{&RunDelay_fs_, 1},
		{&Delay_ps_, 10},
		{&RunHighValueMin_, 1},
		{&RunHighValueMax_, 1},
		{&RunHighValueMinPos_ms_, 100},
		{&RunHighValueMaxPos_ms_, 100},
		{&RunSwitchingMin_, 1},
		{&RunSwitchingMax_, 1},
		{&RunSwitchingMinPos_ms_, 100},
		{&RunSwitchingMaxPos_ms_, 100},
		{&RunMaxBurstLength_, 1},
		{&RunMaxPauseLength_, 1},
		{&RunMaxBurstPos_us_, 100000},
		{&RunMaxPausePos_us_, 100000},
};
static int DynTimeflowPingSize = sizeof(DynTimeflowPing) / sizeof(DynFields_t);
static char TimeflowPing[] = {DATA_OP".FTLightPing#121,"OPERATOR".TimeflowPing"SUBJECT_TEST"#`0`,`00000000`,`-.`°C,`.```V,````.``````ns,``.``ns,`+`,`+`,``.`s,``.`s,`+`,`+`,``.`s,``.`s,`+`,`-``,``.`s,``.`s\n"};

// dynamic web content params
#define CHART_OFFSET_X 40
#define CHART_OFFSET_Y 40
#define CHART_WIDTH 1000
#define CHART_HEIGHT 400
#define LENGTH_VALUE 10
#define LENGTH_TUPLE 9
#define LENGTH_INT3 3
#define CYCLE_LENGTH 1000
#define CYCLE_FIELD 9000
#define STAT_LENGTH 800
#define STAT_FIELD 7200
#define RESPONSE_LENGTH 600
#define RESPONSE_FIELD 5400
#define ZOOM_LENGTH 1000
#define ZOOM_FIELD 9000
#define RUN_FIELD 9000

/* dynamic web response */
const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

DynFields_t DynFields_start[] = {
	{&DeviceID_, 1},
	{&HighValueBeg_px_, 1},
	{&HighValueLen_px_, 1},
	{&HighValueAvgSmooth_px_, 1},
	{&HighValueAvgSmooth_px_, 1},
	{&MaxBurstPos_px_, 1},
	{&MaxBurstPos_px_, 1},
	{&MaxPausePos_px_, 1},
	{&MaxPausePos_px_, 1},
	{&MaxBurstBeg_px_, 1},
	{&MaxBurstLen_px_, 1},
	{&MaxPauseBeg_px_, 1},
	{&MaxPauseLen_px_, 1},
	{&MaxBurstTxt_px_, 1},
	{&MaxBurstLength_, 1},
	{&MaxPauseTxt_px_, 1},
	{&MaxPauseLength_, 1},
	{&ZoomLabelY_, 1},

};
static int DynFieldsSize_start = sizeof(DynFields_start) / sizeof(DynFields_t);
static char html_start[] =
"<html><head><title>Spacetime Waves</title><meta http-equiv=\"refresh\" content=\"4\"></head>\
<body><svg width=\"1050\" height=\"480\">\
<rect width=\"1050\" height=\"480\" style=\"fill:rgb(224,224,224)\"/>\
<!-- headline, frame & version -->\
<text x=\"150\" y=\"25\" style=\"fill:blue;font-size:30\">"OPERATOR".SpacetimeWaveDetectorXMC4700"SUBJECT_TEST"#`0`</text>\
<rect x=\"40\" y=\"40\" width=\"1000\" height=\"400\" style=\"fill:white;stroke-width:1;stroke:rgb(0,0,0)\"/>\
<text x=\"45\" y=\"60\" font=\"Arial\" style=\"fill:black;font-size:15\">"FIRMWARE_VERSION"</text>\
<!-- statistics X -->\
<rect x=\"`0`\" y=\"65\" width=\"`0`\" height=\"75\" style=\"fill:red;stroke:white;stroke-width:0;opacity:0.2\" />\
<line x1=\"`0`\" y1=\"65\" x2=\"`0`\" y2=\"140\" style=\"stroke:red;stroke-width:2\" stroke-dasharray=\"2, 2\" />\
<text x=\"826\" y=\"75\" font=\"Arial\" style=\"fill:green;font-size:13\">2s</text>\
<text x=\"41\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">100</text>\
<line x1=\"140\" y1=\"96\" x2=\"140\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"131\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">200</text>\
<line x1=\"190\" y1=\"65\" x2=\"190\" y2=\"140\" style=\"stroke:red;stroke-width:0.5\" stroke-dasharray=\"5, 5\" />\
<line x1=\"240\" y1=\"96\" x2=\"240\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"231\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">300</text>\
<line x1=\"340\" y1=\"96\" x2=\"340\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"331\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">400</text>\
<line x1=\"440\" y1=\"65\" x2=\"440\" y2=\"140\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"431\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">500</text>\
<text x=\"442\" y=\"75\" font=\"Arial\" style=\"fill:green;font-size:13\">1s</text>\
<line x1=\"540\" y1=\"96\" x2=\"540\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"531\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">600</text>\
<line x1=\"640\" y1=\"96\" x2=\"640\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"631\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">700</text>\
<line x1=\"690\" y1=\"65\" x2=\"690\" y2=\"140\" style=\"stroke:red;stroke-width:0.5\" stroke-dasharray=\"5, 5\" />\
<line x1=\"740\" y1=\"96\" x2=\"740\" y2=\"108\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"731\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">800</text>\
<text x=\"820\" y=\"138\" font=\"Arial\" style=\"fill:red;font-size:12\">900</text>\
<line x1=\"840\" y1=\"65\" x2=\"840\" y2=\"140\" style=\"stroke:black;stroke-width:1\" />\
<!-- statistics Y -->\
<text x=\"3\" y=\"64\" font=\"Arial\" style=\"fill:green;font-size:12\">Stat/ps</text>\
<line x1=\"40\" y1=\"64\" x2=\"840\" y2=\"64\" style=\"stroke:black;stroke-width:1\" />\
<line x1=\"40\" y1=\"76\" x2=\"840\" y2=\"76\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"18\" y=\"80\" font=\"Arial\" style=\"fill:black;font-size:12\">800</text>\
<line x1=\"40\" y1=\"89\" x2=\"840\" y2=\"89\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"18\" y=\"93\" font=\"Arial\" style=\"fill:black;font-size:12\">700</text>\
<line x1=\"40\" y1=\"102\" x2=\"840\" y2=\"102\" style=\"stroke:gray;stroke-width:1\" />\
<text x=\"18\" y=\"106\" font=\"Arial\" style=\"fill:black;font-size:12\">600</text>\
<line x1=\"40\" y1=\"115\" x2=\"840\" y2=\"115\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"18\" y=\"119\" font=\"Arial\" style=\"fill:black;font-size:12\">500</text>\
<line x1=\"40\" y1=\"128\" x2=\"840\" y2=\"128\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"18\" y=\"132\" font=\"Arial\" style=\"fill:black;font-size:12\">400</text>\
<text x=\"1\" y=\"143\" font=\"Arial\" style=\"fill:red;font-size:12\">Respns</text>\
<!-- statistics burst/pause -->\
<line x1=\"`0`\" y1=\"65\" x2=\"`0`\" y2=\"76\" style=\"stroke:darkorange;stroke-width:2\" />\
<line x1=\"`0`\" y1=\"65\" x2=\"`0`\" y2=\"76\" style=\"stroke:olive;stroke-width:2\" />\
<rect x=\"`0`\" y=\"140\" width=\"`0``\" height=\"50\" style=\"fill:darkorange;stroke:white;stroke-width:0;opacity:0.5\" />\
<rect x=\"`0`\" y=\"140\" width=\"`0``\" height=\"50\" style=\"fill:olive;stroke:white;stroke-width:0;opacity:0.5\" />\
<text x=\"`0`\" y=\"186\" font=\"Arial\" style=\"fill:black;font-size:12\">burst (`+`)</text>\
<text x=\"`0`\" y=\"151\" font=\"Arial\" style=\"fill:black;font-size:12\">pause (`-``)</text>\
<!-- cycle -->\
<text x=\"10\" y=\"154\" font=\"Arial\" style=\"fill:black;font-size:12\">20 ns</text>\
<text x=\"4\" y=\"175\" font=\"Arial\" style=\"fill:magenta;font-size:14\">Cycle</text>\
<text x=\"1006\" y=\"186\" font=\"Arial\" style=\"fill:black;font-size:12;opacity:0.5\">2.5ms</text>\
<line x1=\"40\" y1=\"140\" x2=\"1040\" y2=\"140\" style=\"stroke:black;stroke-width:1\" />\
<!-- zoom/run lines -->\
<line x1=\"890\" y1=\"190\" x2=\"890\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<line x1=\"740\" y1=\"190\" x2=\"740\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<line x1=\"590\" y1=\"190\" x2=\"590\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<line x1=\"440\" y1=\"190\" x2=\"440\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<line x1=\"290\" y1=\"190\" x2=\"290\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<line x1=\"140\" y1=\"190\" x2=\"140\" y2=\"440\" style=\"stroke:gray;stroke-width:0.5\" />\
<!-- zoom -->\
<text x=\"4\" y=\"196\" font=\"Arial\" style=\"fill:black;font-size:12\">`0``ps</text>\
<text x=\"7\" y=\"212\" font=\"Arial\" style=\"fill:olive;font-size:12\">Pause</text>\
<text x=\"13\" y=\"236\" font=\"Arial\" style=\"fill:blue;font-size:12\">Max</text>\
<text x=\"15\" y=\"261\" font=\"Arial\" style=\"fill:black;font-size:12\">Avg</text>\
<text x=\"14\" y=\"286\" font=\"Arial\" style=\"fill:red;font-size:12\">Min</text>\
<text x=\"4\" y=\"312\" font=\"Arial\" style=\"fill:black;font-size:14\">Zoom</text>\
<text x=\"8\" y=\"332\" font=\"Arial\" style=\"fill:darkorange;font-size:12\">Burst</text>\
<line x1=\"40\" y1=\"190\" x2=\"1040\" y2=\"190\" style=\"stroke:black;stroke-width:1\" />\
<line x1=\"40\" y1=\"258\" x2=\"1040\" y2=\"258\" style=\"stroke:gray;stroke-width:0.5\" />\
<text x=\"1031\" y=\"322\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">0</text>\
<text x=\"885\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-5</text>\
<text x=\"730\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-10</text>\
<text x=\"580\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-15</text>\
<text x=\"430\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-20</text>\
<text x=\"280\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-25</text>\
<text x=\"130\" y=\"320\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">-30</text>\
<text x=\"965\" y=\"313\" font=\"Arial\" style=\"fill:black;font-size:13;opacity:0.5\">time [min]</text>\
<!-- run -->\
<text x=\"0\" y=\"346\" font=\"Arial\" style=\"fill:black;font-size:12\">1200 ps</text>\
<text x=\"7\" y=\"360\" font=\"Arial\" style=\"fill:olive;font-size:12\">Pause</text>\
<text x=\"13\" y=\"380\" font=\"Arial\" style=\"fill:blue;font-size:12\">Max</text>\
<text x=\"15\" y=\"392\" font=\"Arial\" style=\"fill:black;font-size:12\">Avg</text>\
<text x=\"14\" y=\"404\" font=\"Arial\" style=\"fill:red;font-size:12\">Min</text>\
<text x=\"4\" y=\"422\" font=\"Arial\" style=\"fill:black;font-size:14\">Run</text>\
<text x=\"8\" y=\"436\" font=\"Arial\" style=\"fill:darkorange;font-size:12\">Burst</text>\
<line x1=\"40\" y1=\"340\" x2=\"1040\" y2=\"340\" style=\"stroke:black;stroke-width:1\" />\
<text x=\"1033\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">0</text>\
<text x=\"885\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-4</text>\
<text x=\"730\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-8</text>\
<text x=\"580\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-12</text>\
<text x=\"430\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-16</text>\
<text x=\"280\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-20</text>\
<text x=\"130\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">-24</text>\
<text x=\"980\" y=\"455\" font=\"Arial\" style=\"fill:black;font-size:13\">time [h]</text>\
<!-- stat curve -->\
<polyline style=\"fill:none;stroke:green;stroke-width:1\" points=\"";
static char html_stat[STAT_FIELD];
static char html_stat_end[] =
"\"/>\
<!-- response curve -->\
<polyline style=\"fill:none;stroke:red;stroke-width:1\" points=\"";
static char html_response[RESPONSE_FIELD];
static char html_response_end[] =
"\"/>\
<!-- cycle curve -->\
<polyline style=\"fill:none;stroke:magenta;stroke-width:1\" points=\"";
static char html_cycle[CYCLE_FIELD];
static char html_cycle_end[] =
"\"/>\
<!-- zoom avg curve -->\
<polyline style=\"fill:none;stroke:black;stroke-width:1\" points=\"";
static char html_zoom_avg[ZOOM_FIELD];
static char html_zoom_avg_end[] =
"\"/>\
<!-- zoom min curve -->\
<polyline style=\"fill:none;stroke:red;stroke-width:1\" points=\"";
static char html_zoom_min[ZOOM_FIELD];
static char html_zoom_min_end[] =
"\"/>\
<!-- zoom max curve -->\
<polyline style=\"fill:none;stroke:blue;stroke-width:1\" points=\"";
static char html_zoom_max[ZOOM_FIELD];
static char html_zoom_max_end[] =
"\"/>\
<!-- zoom burst curve -->\
<polyline style=\"fill:none;stroke:darkgoldenrod;stroke-width:1\" points=\"";
static char html_zoom_burst[ZOOM_FIELD];
static char html_zoom_burst_end[] =
"\"/>\
<!-- zoom pause curve -->\
<polyline style=\"fill:none;stroke:olive;stroke-width:1\" points=\"";
static char html_zoom_pause[ZOOM_FIELD];
static char html_zoom_pause_end[] =
"\"/>\
<!-- run avg curve -->\
<polyline style=\"fill:none;stroke:gray;stroke-width:1\" points=\"";
static char html_run_avg[RUN_FIELD];
static char html_run_avg_end[] =
"\"/>\
<!-- run min curve -->\
<polyline style=\"fill:none;stroke:red;stroke-width:1\" points=\"";
static char html_dummy[RUN_FIELD]; // workaround for website problem @605, 39< 606
static char html_run_min[RUN_FIELD];
static char html_run_min_end[] =
"\"/>\
<!-- run max curve -->\
<polyline style=\"fill:none;stroke:blue;stroke-width:1\" points=\"";
static char html_run_max[RUN_FIELD];
static char html_run_max_end[] =
"\"/>\
<!-- run burst curve -->\
<polyline style=\"fill:none;stroke:darkgoldenrod;stroke-width:1\" points=\"";
static char html_run_burst[RUN_FIELD];
static char html_run_burst_end[] =
"\"/>\
<!-- run pause curve -->\
<polyline style=\"fill:none;stroke:olive;stroke-width:1\" points=\"";
static char html_run_pause[RUN_FIELD];

DynFields_t DynFields_end[] = {
		{&Delay_fs_, 1},
		{&Phase_deg_, 1},
		{&DelayStep_, 1},
		{&Delay_ps_, 10},
		{&Inherent_ps_, 10},
		{&Temperature_, 1},
		{&Ucc_, 1000},
		{&Ucc5V_, 1000},
		{&Cycles_, 1},
		{&SwitchingAvg_, 1},
		{&HighValueMin_, 1},
		{&HighValueMax_, 1},
		{&HighValueAvgSmooth_, 1000},
};
static int DynFieldsSize_end = sizeof(DynFields_end) / sizeof(DynFields_t);
static char html_end[] =
"\"/>\
<!-- numerical values -->\
<text x=\"115\" y=\"60\" font=\"Arial\" style=\"fill:black;font-size:24\">Line delay: ````.`````` ns</text>\
<text x=\"433\" y=\"60\" font=\"Arial\" style=\"fill:black;font-size:20\">Tuning: ```.`&deg; (#`0`: ```.`` + fix: ``.`` ns)</text>\
<text x=\"848\" y=\"60\" font=\"Arial\" style=\"fill:black;font-size:15\">\
<tspan x=\"846\" y=\"56\">Temperature:</tspan>\
<tspan x=\"960\" y=\"56\">`-.`&deg;C</tspan>\
<tspan x=\"846\" y=\"76\">Voltage:</tspan>\
<tspan x=\"915\" y=\"76\" style=\"font-size:10\">`.``` V</tspan>\
<tspan x=\"960\" y=\"76\">`.``` V</tspan>\
<tspan x=\"846\" y=\"96\">Cycles [mil]:</tspan>\
<tspan x=\"960\" y=\"96\">````.``````</tspan>\
<tspan x=\"846\" y=\"116\">High/Switch:</tspan>\
<tspan x=\"988\" y=\"116\">/ `+`</tspan>\
<tspan x=\"846\" y=\"136\">High min/max:</tspan>\
<tspan x=\"960\" y=\"136\" style=\"fill:red\">`+`` ... `+``</tspan></text>\
<text x=\"848\" y=\"60\" font=\"Arial\" style=\"fill:red;font-size:15\">\
<tspan x=\"952\" y=\"116\">~`+`</tspan></text>";

DynFields_t DynFields_telemetry[] = {
		{&Solar_, 10000},
		{&Battery_, 10000},
		{&Power_, 10000},
		{&PoE_, 10000},
		{&U_cycle_, 1},
		{&U_delay_, 1},
};
static int DynFieldsSize_telemetry = sizeof(DynFields_telemetry) / sizeof(DynFields_t);
static char html_telemetry[] =
"<text x=\"45\" y=\"475\" font=\"Arial\" style=\"fill:darkred;font-size:13\">Solar: `-`.``V</text>\
<text x=\"160\" y=\"475\" font=\"Arial\" style=\"fill:darkcyan;font-size:13\">Battery: `-`.``V</text>\
<text x=\"310\" y=\"475\" font=\"Arial\" style=\"fill:darkcyan;font-size:13\">Power: ``.``W</text>\
<text x=\"460\" y=\"475\" font=\"Arial\" style=\"fill:darkblue;font-size:13\">PoE: ``.``V</text>\
<text x=\"610\" y=\"475\" font=\"Arial\" style=\"fill:darkblue;font-size:13\">U_cycle: `0``</text>\
<text x=\"760\" y=\"475\" font=\"Arial\" style=\"fill:darkblue;font-size:13\">U_delay: `0``</text>";

static char html_close[] =
"</svg></body></html>";

// Int3 array
#define INT3MAX 999
static char Int3Plot[3*(INT3MAX+1)];

// measurement arrays
static int16_t StatPlot[STAT_LENGTH];
static int16_t ResponsePlot[RESPONSE_LENGTH];
static int16_t CyclePlot[CYCLE_LENGTH];
static int16_t ZoomAvgPlot[ZOOM_LENGTH];
static int16_t ZoomMinPlot[ZOOM_LENGTH];
static int16_t ZoomMaxPlot[ZOOM_LENGTH];
static int16_t ZoomBurstPlot[ZOOM_LENGTH];
static int16_t ZoomPausePlot[ZOOM_LENGTH];
static int16_t RunAvgPlot[RUN_LENGTH];
static int16_t RunMinPlot[RUN_LENGTH];
static int16_t RunMaxPlot[RUN_LENGTH];
static int16_t RunBurstPlot[RUN_LENGTH];
static int16_t RunPausePlot[RUN_LENGTH];

bool sendTimeflowCalibrationMessage(struct netconn *_conn)
{
    // update telemetry values
    updateDynamicFields(DynTimeflowCalibration, DynTimeflowCalibrationSize);

    // send calibration message
	if( ERR_OK != netconn_write(_conn, TimeflowCalibration, strlen(TimeflowCalibration),NETCONN_NOCOPY)){
		return false;	// failed
	}
	// success
	return true;
}

bool sendTimeflowSolarPowerMessage(struct netconn *_conn)
{
    // update timeflow solar power values
    updateDynamicFields(DynTimeflowSolarPower, DynTimeflowSolarPowerSize);

    // send timeflow solar power message
	if( ERR_OK != netconn_write(_conn, TimeflowSolarPower, strlen(TimeflowSolarPower),NETCONN_NOCOPY)){
		return false;	// failed
	}
	// success
	return true;
}

bool sendTimeflowPing(struct netconn *_conn)
{
	// PING_TEST data for test device #50
	if(PING_TEST && (DEVICE_ID == 50)){
		Delay_fs_ = 2200500000;
		RunHighValueMin_ = 400;
		RunHighValueMax_ = 500;
		RunHighValueMinPos_ms_ = 40;
		RunHighValueMaxPos_ms_ = 50;
		RunSwitchingMin_ = 600;
		RunSwitchingMax_ = 700;
		RunSwitchingMinPos_ms_ = 60;
		RunSwitchingMaxPos_ms_ = 70;
		RunMaxBurstLength_ = 10;
		RunMaxPauseLength_ = 20;
		RunMaxBurstPos_us_ = 30;
		RunMaxPausePos_us_ = 40;
	}
	PingID_++;

    // update timeflow values
    updateDynamicFields(DynTimeflowPing, DynTimeflowPingSize);

    // send telemetry message
	if( ERR_OK != netconn_write(_conn, TimeflowPing, strlen(TimeflowPing),NETCONN_NOCOPY)){
		return false;	// failed
	}
	// success
	return true;
}

void sendHtmlResponse(struct netconn *_conn)
{
//    /* runtime */
//    int Runtime = TotalCycles_;

    /* Send the HTML header */
    netconn_write(_conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);

    /* update numerical values on HTML page */
    updateDynamicFields(DynFields_start, DynFieldsSize_start);
    updateDynamicFields(DynFields_end, DynFieldsSize_end);

    /* update stat values @ 75-93.75% */
    int StatMin = - 12 * STAT_RANGE/3;
    int StatMax = 4 * STAT_RANGE/3;
    int StatOffsetX = 0;
    updateMeasurement(html_stat, STAT_FIELD, StatPlot_, StatLength_, StatMin, StatMax, StatOffsetX);

    /* update response values @ 75-93.75% */
    int ResponseMin = - 12 * STAT_RANGE/3;
    int ResponseMax = 4 * STAT_RANGE/3;
    int ResponseOffsetX = 100;
    updateResponse(ResponsePlot_, ResponseLength_, STAT_RANGE);
    updateMeasurement(html_response, RESPONSE_FIELD, ResponsePlot_, ResponseLength_, ResponseMin, ResponseMax, ResponseOffsetX);

    /* update cycle values @ 62.5-75% */
    int CycleHigh = CyclePlot_[0];
    for(int i=1; i<CycleLength_; i++){
    	CyclePlot_[i] > CycleHigh ? CycleHigh = CyclePlot_[i] : 0;
    	if(CyclePlot_[i] < CycleHigh)break;
    }
    int CycleMin = CycleHigh - (7 * CYCLE_RANGE)/10;
    int CycleMax = CycleHigh + (3 * CYCLE_RANGE)/10;
    int CycleOffsetX = 0;
    updateMeasurement(html_cycle, CYCLE_FIELD, CyclePlot_, CycleLength_, CycleMin, CycleMax, CycleOffsetX);

    /* update zoom values @ 25-62.5% */
    int ZoomMin = - 2 * ZOOM_RANGE/3;
    int ZoomMax = 6 * ZOOM_RANGE/3;
    int ZoomOffsetX = 0;
    updateMeasurement(html_zoom_avg, ZOOM_FIELD, ZoomAvgPlot_, ZoomLength_, ZoomMin, ZoomMax, ZoomOffsetX);
    updateMeasurement(html_zoom_min, ZOOM_FIELD, ZoomMinPlot_, ZoomLength_, ZoomMin, ZoomMax, ZoomOffsetX);
    updateMeasurement(html_zoom_max, ZOOM_FIELD, ZoomMaxPlot_, ZoomLength_, ZoomMin, ZoomMax, ZoomOffsetX);
    updateMeasurement(html_zoom_burst, ZOOM_FIELD, ZoomBurstPlot_, ZoomLength_, ZoomMin, ZoomMax, ZoomOffsetX);
    updateMeasurement(html_zoom_pause, ZOOM_FIELD, ZoomPausePlot_, ZoomLength_, ZoomMin, ZoomMax, ZoomOffsetX);

    /* update run values @ 0-25% */
    int RunMin = 0;
    int RunMax = 4 * RUN_RANGE;
    int RunOffsetX = 0;
    updateMeasurement(html_run_avg, RUN_FIELD, RunAvgPlot_, RunLength_, RunMin, RunMax, RunOffsetX);
    updateMeasurement(html_run_min, RUN_FIELD, RunMinPlot_, RunLength_, RunMin, RunMax, RunOffsetX);
    updateMeasurement(html_run_max, RUN_FIELD, RunMaxPlot_, RunLength_, RunMin, RunMax, RunOffsetX);
    updateMeasurement(html_run_burst, RUN_FIELD, RunBurstPlot_, RunLength_, RunMin, RunMax, RunOffsetX);
    updateMeasurement(html_run_pause, RUN_FIELD, RunPausePlot_, RunLength_, RunMin, RunMax, RunOffsetX);
    /* send subsequent parts of the html page */
    netconn_write(_conn, html_start, sizeof(html_start)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_stat, sizeof(html_stat)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_stat_end, sizeof(html_stat_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_response, sizeof(html_response)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_response_end, sizeof(html_response_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_cycle, sizeof(html_cycle)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_cycle_end, sizeof(html_cycle_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_avg, sizeof(html_zoom_avg)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_avg_end, sizeof(html_zoom_avg_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_min, sizeof(html_zoom_min)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_min_end, sizeof(html_zoom_min_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_max, sizeof(html_zoom_max)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_max_end, sizeof(html_zoom_max_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_burst, sizeof(html_zoom_burst)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_burst_end, sizeof(html_zoom_burst_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_pause, sizeof(html_zoom_pause)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_zoom_pause_end, sizeof(html_zoom_pause_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_avg, sizeof(html_run_avg)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_avg_end, sizeof(html_run_avg_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_min, sizeof(html_run_min)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_min_end, sizeof(html_run_min_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_max, sizeof(html_run_max)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_max_end, sizeof(html_run_max_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_burst, sizeof(html_run_burst)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_burst_end, sizeof(html_run_burst_end)-1, NETCONN_NOCOPY);
    netconn_write(_conn, html_run_pause, sizeof(html_run_pause)-1, NETCONN_NOCOPY);

//    /* show runtime as monitor value */
//    Runtime = 5 * (TotalCycles_ - Runtime)/2;  // runtime [ms]
//    updateValue(html_end + POS_MONITOR, LENGTH_VALUE, Runtime, 0, isNegative=true);

    netconn_write(_conn, html_end, sizeof(html_end)-1, NETCONN_NOCOPY);

    // telemetry
    if(TELEMETRY){
    	updateDynamicFields(DynFields_telemetry, DynFieldsSize_telemetry);
        netconn_write(_conn, html_telemetry, sizeof(html_telemetry)-1, NETCONN_NOCOPY);
    }

    // close html
    netconn_write(_conn, html_close, sizeof(html_close)-1, NETCONN_NOCOPY);

    // TEST
    html_dummy[5092] = '0';
}

void initDynamicWebContent()
{
	// params
	DeviceID_ = DEVICE_ID;
	PingID_ = 0;
	int PosX;
	int Point = 0;
	bool isLeftAligned = true;
	bool isLeadingZero = false;
	bool isNegative = false;

	// fill Int3 array
	for(int i=0; i<=INT3MAX; i++){
		updateValue(Int3Plot + LENGTH_INT3*i, LENGTH_INT3, i, Point, isLeftAligned, isLeadingZero, isNegative);
	}

	// fill stat
	for(int i=0; i<STAT_LENGTH; i++){
		StatPlot[i] = STAT_RANGE/2;
		PosX = CHART_OFFSET_X + i;
		updateValue(html_stat + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		html_stat[LENGTH_TUPLE*i+4] = ',';
	}

	// fill response
	int ResponseOffsetX = 100;
	for(int i=0; i<RESPONSE_LENGTH; i++){
		ResponsePlot[i] = (STAT_RANGE * i) / RESPONSE_LENGTH;
		PosX = CHART_OFFSET_X + ResponseOffsetX + i;
		updateValue(html_response + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		html_response[LENGTH_TUPLE*i+4] = ',';
	}

	// fill cycle
	for(int i=0; i<CYCLE_LENGTH; i++){
		CyclePlot[i] = CYCLE_RANGE/2;
		PosX = CHART_OFFSET_X + i;
		updateValue(html_cycle + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		html_cycle[LENGTH_TUPLE*i+4] = ',';
	}

	// fill zoom
	for(int i=0; i<ZOOM_LENGTH; i++){
		ZoomAvgPlot[i] = ZOOM_RANGE/2;
		ZoomMinPlot[i] = ZOOM_RANGE/2;
		ZoomMaxPlot[i] = ZOOM_RANGE/2;
		ZoomBurstPlot[i] = ZOOM_RANGE/2;
		ZoomPausePlot[i] = ZOOM_RANGE/2;
		PosX = CHART_OFFSET_X + i;
		updateValue(html_zoom_avg + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_zoom_min + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_zoom_max + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_zoom_burst + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_zoom_pause + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		html_zoom_avg[LENGTH_TUPLE*i+4] = ',';
		html_zoom_min[LENGTH_TUPLE*i+4] = ',';
		html_zoom_max[LENGTH_TUPLE*i+4] = ',';
		html_zoom_burst[LENGTH_TUPLE*i+4] = ',';
		html_zoom_pause[LENGTH_TUPLE*i+4] = ',';
	}

	// fill run
	for(int i=0; i<RUN_LENGTH; i++){
		RunAvgPlot[i] = RUN_RANGE/2;
		RunMinPlot[i] = RUN_RANGE/2;
		RunMaxPlot[i] = RUN_RANGE/2;
		RunBurstPlot[i] = RUN_RANGE/2;
		RunPausePlot[i] = RUN_RANGE/2;
		PosX = CHART_OFFSET_X + i;
		updateValue(html_run_avg + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_run_min + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_run_max + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_run_burst + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		updateValue(html_run_pause + LENGTH_TUPLE*i, LENGTH_TUPLE, PosX, Point, isLeftAligned, isLeadingZero, isNegative);
		html_run_avg[LENGTH_TUPLE*i+4] = ',';
		html_run_min[LENGTH_TUPLE*i+4] = ',';
		html_run_max[LENGTH_TUPLE*i+4] = ',';
		html_run_burst[LENGTH_TUPLE*i+4] = ',';
		html_run_pause[LENGTH_TUPLE*i+4] = ',';
	}

	// publish stat array
	StatPlot_ = StatPlot;
	StatLength_ = STAT_LENGTH;

	// publish response array
	ResponsePlot_ = ResponsePlot;
	ResponseLength_ = RESPONSE_LENGTH;

	// publish cycle array
	CyclePlot_ = CyclePlot;
	CycleLength_ = CYCLE_LENGTH;

	// publish zoom array
	ZoomLength_ = ZOOM_LENGTH;
	ZoomAvgPlot_ = ZoomAvgPlot;
	ZoomMinPlot_ = ZoomMinPlot;
	ZoomMaxPlot_ = ZoomMaxPlot;
	ZoomBurstPlot_ = ZoomBurstPlot;
	ZoomPausePlot_ = ZoomPausePlot;

	// publish run arrays
	RunLength_ = RUN_LENGTH;
	RunAvgPlot_ = RunAvgPlot;
	RunMinPlot_ = RunMinPlot;
	RunMaxPlot_ = RunMaxPlot;
	RunBurstPlot_ = RunBurstPlot;
	RunPausePlot_ = RunPausePlot;

	// parse further dynamic fields on web page
	parseDynamicFields(html_start, sizeof(html_start),DynFields_start, DynFieldsSize_start);
	parseDynamicFields(html_end, sizeof(html_end),DynFields_end, DynFieldsSize_end);
	parseDynamicFields(html_telemetry, sizeof(html_telemetry),DynFields_telemetry, DynFieldsSize_telemetry);

	// parse timeflow calibration message fields
	parseDynamicFields(TimeflowCalibration, sizeof(TimeflowCalibration),DynTimeflowCalibration, DynTimeflowCalibrationSize);

	// parse timeflow solar power message fields
	parseDynamicFields(TimeflowSolarPower, sizeof(TimeflowSolarPower),DynTimeflowSolarPower, DynTimeflowSolarPowerSize);

	// parse timeflow ping fields
	parseDynamicFields(TimeflowPing, sizeof(TimeflowPing),DynTimeflowPing, DynTimeflowPingSize);
}

void parseDynamicFields(char *_HtmlFile, int _HtmlFileLength, DynFields_t* _DynFields, int _DynFieldsSize)
{
	// parse html for dynamic content
	bool isDynField = false;
	for(int i=0; i<_HtmlFileLength; i++){
		if(isDynField){
			// analyse field format
			switch(_HtmlFile[i]){
			case '`':
				break;
			case '0':
				_DynFields->isNumeric = true;
				_DynFields->isLeadingZero = true;
				break;
			case '+':
				_DynFields->isNumeric = true;
				break;
			case '-':
				_DynFields->isNegative = true;
				_DynFields->isNumeric = true;
				break;
			case '.':
				_DynFields->FractionLength = i;
				_DynFields->isNumeric = true;
				break;
			default:
				isDynField = false;
				_DynFields->HtmlFieldLength = _HtmlFile+i - _DynFields->HtmlField;
				_DynFields->FractionLength > 0 ? _DynFields->FractionLength = i - _DynFields->FractionLength - 1 : 0;
				_DynFields += 1;
				_DynFieldsSize--;
				break;
			}
		}else{
			// look for start of a dynamic field
			if(_HtmlFile[i] == '`'){
				isDynField = true;
				_DynFields->isNumeric = false;
				_DynFields->isNegative = false;
				_DynFields->isLeftAligned = true;
				_DynFields->isLeadingZero = false;
				_DynFields->HtmlField = _HtmlFile+i;
				_DynFields->HtmlFieldLength = 0;
				_DynFields->FractionLength = 0;
			}
		}
		// check for end of DynFields
		if(_DynFieldsSize <= 0){
			break;
		}
	}
}

void updateDynamicFields(DynFields_t* _DynFields, int _DynFieldsSize)
{
	// iterate all dynamic fields
	for(; _DynFieldsSize>0; _DynFieldsSize--){
		// update a particular field
		if(_DynFields->isNumeric){
			// numeric field
			if(_DynFields->Divisor == 1){
				updateValue(_DynFields->HtmlField, _DynFields->HtmlFieldLength, *_DynFields->Value, _DynFields->FractionLength, _DynFields->isLeftAligned, _DynFields->isLeadingZero, _DynFields->isNegative);
			}else if(_DynFields->Divisor != 0){
				updateValue(_DynFields->HtmlField, _DynFields->HtmlFieldLength, *_DynFields->Value/_DynFields->Divisor, _DynFields->FractionLength, _DynFields->isLeftAligned, _DynFields->isLeadingZero, _DynFields->isNegative);
			}
		}else{
			// text field

		}
		_DynFields++;
	}

}

void updateValueInt3(char *_Field, int _ValueInt3)
{
	// verify value
	_ValueInt3 < 0 ? _ValueInt3 = 0 : _ValueInt3 > INT3MAX ? _ValueInt3 = INT3MAX : 0;

	// update a 3-digit int value from pre-calculated array
	memcpy(_Field, Int3Plot + LENGTH_INT3*_ValueInt3, LENGTH_INT3);
}

void updateValue(char *_Field, int _Length, int _Value, int _Point, bool _isLeftAligned, bool _isLeadingZero, bool _isNegative)
{
	// initialize field with zero or with space respectively
	_isLeadingZero ? memset(_Field, '0', _Length) : memset(_Field, ' ', _Length);

	// handle negative values
	if(_isNegative && (_Value < 0)){
		_Value = -_Value;
		*_Field = '-';
		_Field++;
		_Length--;
	}

#define MAX_LENGTH 10
	char Text[MAX_LENGTH+1];
	memset(Text,0,sizeof(Text));
	sprintf(Text, "%u", _Value);
	int Length = strlen(Text);
	Length > _Length ?  Length = _Length : 0;
	_Point > 0 ? Length = Length > _Point ? Length - _Point : 0 : 0;
	if(_isLeadingZero){
		Length > 0 ? memcpy(_Field + _Length - Length, Text, Length) : 0;
	}else if(_isLeftAligned || _Point > 0){
		Length > 0 ? memcpy(_Field, Text, Length) : 0;
	}else{
		Length > 0 ? memcpy(_Field + _Length - Length, Text, Length) : 0;
	}
	if(_Point > 0 && Length < _Length){
		Length + _Point >= _Length ? _Point = _Length - Length - 1 : 0;
		*(_Field+Length) = '.';
		if(_Point > 0){
			memset(_Field+Length+1, '0', _Point);
			int Fraction = strlen(Text) - Length;
			Fraction > _Point ? Fraction = _Point : 0;
			memcpy(_Field+Length+_Point-Fraction+1, Text+Length, Fraction);
		}
	}
}

void updateMeasurement(char *_Field, int _FieldLength, int16_t *_Buffer, int _BufferLength, int _ValueMin, int _ValueMax, int _OffsetX)
{
	// fill buffer with measurement values
	int Value, Range, PosY;
	Range = _ValueMax - _ValueMin;
	for(int i=0; i<_BufferLength; i++){
		Value = _Buffer[i] < _ValueMin ? _ValueMin : _Buffer[i] > _ValueMax ? _ValueMax : _Buffer[i];
		PosY = CHART_OFFSET_Y + (CHART_HEIGHT * (_ValueMax - Value)) / Range;
		updateValueInt3(_Field + LENGTH_TUPLE*i + 5, PosY);
	}
}
