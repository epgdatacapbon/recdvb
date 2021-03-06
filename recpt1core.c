#include <dirent.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "recpt1core.h"
#include "pt1_dev.h"

#define ISDB_T_NODE_LIMIT 24  // 32:ARIB limit 24:program maximum
#define ISDB_T_SLOT_LIMIT 8

#ifndef DTV_STREAM_ID
#define DTV_STREAM_ID DTV_ISDBS_TS_ID
#endif

#define DMX_BUFFER_SIZE 1024 * 1024

/* globals */
const char *version = PACKAGE_VERSION;
boolean f_exit = FALSE;
ISDB_FREQ_CONV_TABLE isdb_conv_table[MAX_CH];
CHANNEL_SET channel_set;

// 機種依存用
#define OTHER_TUNER 0x0000  // 未知
#define ISDBS_TUNER 0x0001  // ISDB-S(衛星チューナー)フラグ
#define EARTH_PT1 0x0002    // PT1 PT2
#define EARTH_PT3 0x0004    // PT3 PX-BCUD(?)
#define SIANO_MBL 0x0008    // PX-S1UD S880
#define DIBCOM8000 0x0010   // S870
#define FRIIO_WT 0x0020     // Friio白
int tuner_type = OTHER_TUNER;
struct {
	char *name;
	int type;
} tuners_prop[] = {
	{"VA1J5JF8007/VA1J5JF8011 ISDB-", EARTH_PT1},
	{"Toshiba TC90522 ISDB-", EARTH_PT3},
	{"774 Friio ISDB-T USB2.0", FRIIO_WT},
	{"Siano Mobile Digital MDTV Receiver", SIANO_MBL},
	{"DiBcom 8000 ISDB-T", DIBCOM8000},
	{NULL, 0}};

void modify_ch_str(char *channel)
{
	char buf[16];
	char *p1 = channel;
	char *p2 = buf;

	if (!p1 || *p1 == '\0')
		return;

	if (*p1 == '0' && (*(p1 + 1) == 'X' || *(p1 + 1) == 'x')) {
		*p2++ = '0';
		*p2++ = 'x';
		p1 += 2;
	}

	while (!isdigit(*p1) && *p1 != '\0')
		*p2++ = toupper(*p1++);
	*p2 = '\0';

	while (*p1 == '0')
		p1++;

	strcat(p2, p1);
	strcpy(channel, buf);
}

static unsigned int GetFrequency_S(int ch)
{
	unsigned int freq;
	if (ch < 12)
		freq = 1049480 + 38360 * ch;
	else if (ch < 24)
		freq = 1613000 + 40000 * (ch - 12);
	else if (ch < 36)
		freq = 1593000 + 40000 * (ch - 24);
	else
		freq = 1049480;  // 変な値なので0chに
	return freq;
}

static unsigned int GetFrequency_T(int ch)
{
	unsigned int freq;
	if (ch < 12)
		freq = 93142857 + 6000000 * ch;
	else if (ch < 17)
		freq = 167142857 + 6000000 * (ch - 12);
	else if (ch < 63)
		freq = 195142857 + 6000000 * (ch - 17);
	else if (ch < 113)
		freq = 473142857 + 6000000 * (ch - 63);
	else
		freq = 93142857;  // 変な値なので0chに
	return freq;
}

CHANNEL_SET *searchrecoff(char *channel)
{
	char ch_str[16];
	int i = 0, ch_no;

	if (!channel || *channel == '\0' || strlen(channel) > 15)
		return NULL;

	strcpy(ch_str, channel);
	modify_ch_str(ch_str);
	ch_no = (int)strtol(ch_str, NULL, 0);
	if ((ch_no > 0 && ch_no < 133) || (ch_str[0] == 'C' && isdigit(ch_str[1]))) {
		if (ch_no == 0) {  // CATV
			ch_no = (int)strtol(&ch_str[1], NULL, 10);
			if (ch_no > 12 && ch_no <= 22)
				ch_no -= 10;
			else if (ch_no > 22 && ch_no <= 63)
				ch_no--;
			else
				return NULL;
		} else if (ch_no < 4)
			ch_no--;
		else if (ch_no < 13)
			ch_no += 9;
		else
			ch_no += 50;

		channel_set.freq_no = ch_no;
		channel_set.tsid = 0;
		channel_set.type = CHTYPE_GROUND;
		strcpy(channel_set.parm_freq, ch_str);
		return &channel_set;
	} else if (ch_no >= 0x4000 && ch_no <= 0x7fff) {
		while (isdb_conv_table[i].tsid > 0) {
			if (ch_no == isdb_conv_table[i].tsid) {
				channel_set.freq_no = isdb_conv_table[i].freq_no;
				channel_set.tsid = isdb_conv_table[i].tsid;
				channel_set.type = CHTYPE_SATELLITE;
				sprintf(channel_set.parm_freq, "0x%X", ch_no);
				return &channel_set;
			}
			i++;
		}
	} else {
		while (isdb_conv_table[i].channel[0] != '\0') {
			if (!strcmp(ch_str, isdb_conv_table[i].channel)) {
				channel_set.freq_no = isdb_conv_table[i].freq_no;
				channel_set.tsid = isdb_conv_table[i].tsid;
				channel_set.type = CHTYPE_SATELLITE;
				strcpy(channel_set.parm_freq, ch_str);
				return &channel_set;
			}
			i++;
		}
	}

	return NULL;
}

int set_frequency(thread_data *tdata, boolean msg_view)
{
	struct dtv_property prop[3];
	struct dtv_properties props;
	struct dvb_frontend_info fe_info;
	int i;

	if ((ioctl(tdata->fefd, FE_GET_INFO, &fe_info) < 0)) {
		fprintf(stderr, "FE_GET_INFO failed\n");
		return 1;
	}
	// 機種判別
	for (i = 0; tuners_prop[i].name != NULL; i++) {
		if (strncmp(fe_info.name, tuners_prop[i].name, strlen(tuners_prop[i].name)) == 0)
			tuner_type = tuners_prop[i].type;
	}

	if (tdata->table->type == CHTYPE_GROUND) {
		if (fe_info.type != FE_OFDM) {
			if (msg_view)
				fprintf(stderr, "tuner is not UHF(FE_OFDM)\n");
			return 1;
		}
		fprintf(stderr, "Using DVB device \"%s\"\n", fe_info.name);

		prop[0].cmd = DTV_FREQUENCY;
		prop[0].u.data = GetFrequency_T(tdata->table->freq_no);
		prop[1].cmd = DTV_TUNE;
		props.props = prop;
		props.num = 2;
		fprintf(stderr, "tuning to %.3f MHz\n", (double)prop[0].u.data / 1000000);
	} else if (tdata->table->type == CHTYPE_SATELLITE) {
		if (fe_info.type != FE_QPSK) {
			if (msg_view)
				fprintf(stderr, "tuner is not BS/CS110(FE_QPSK)\n");
			return 1;
		}
		fprintf(stderr, "Using DVB device \"%s\"\n", fe_info.name);
		tuner_type |= ISDBS_TUNER;

		if (0 <= tdata->lnb && tdata->lnb <= 2) {
			prop[0].cmd = DTV_VOLTAGE;
			prop[0].u.data = tdata->lnb;
			props.props = prop;
			props.num = 1;
			if (ioctl(tdata->fefd, FE_SET_PROPERTY, &props) < 0)
				fprintf(stderr, "LNB control failed");
		}

		prop[0].cmd = DTV_FREQUENCY;
		prop[0].u.data = GetFrequency_S(tdata->table->freq_no);
		prop[1].cmd = DTV_STREAM_ID;
		prop[1].u.data = tdata->table->tsid;
		prop[2].cmd = DTV_TUNE;
		props.props = prop;
		props.num = 3;
		fprintf(stderr, "tuning to %d MHz\n", prop[0].u.data / 1000);
	}
	if (ioctl(tdata->fefd, FE_SET_PROPERTY, &props) == -1) {
		perror("ioctl FE_SET_PROPERTY\n");
		return 1;
	}
	return 0;
}

int open_tuner(thread_data *tdata, int dev_num, boolean msg_view)
{
	char device[32];

	if (tdata->fefd == 0) {
		sprintf(device, "/dev/dvb/adapter%d/frontend0", dev_num);
		tdata->fefd = open(device, O_RDWR);
		if (tdata->fefd < 0) {
			if (msg_view)
				fprintf(stderr, "cannot open frontend device\n");
			tdata->fefd = 0;
			return 1;
		}
		fprintf(stderr, "device = %s\n", device);
	}
	if (set_frequency(tdata, msg_view)) {
		close(tdata->fefd);
		tdata->fefd = 0;
		return 1;
	} else
		return 0;
}

int dvb_lock_check(thread_data *tdata)
{
	struct timespec ts;

	ts.tv_sec = 0;
	// from BonDriver_DVB.cpp
	// LOCK後しばらくはエラーが交じるため応答性が悪くても良い
	fe_status_t status;
	int i;

	ts.tv_nsec = 250 * 1000 * 1000;  // 250ms
	for (i = 0; i < 12; i++)         // 250ms * 12で最大3秒待つ 3秒待ってもFE_HAS_LOCK後はstatus=0x13で変化無し
	{
		fprintf(stderr, ".");
		if (ioctl(tdata->fefd, FE_READ_STATUS, &status) < 0) {
			fprintf(stderr, "dvb_lock_check() ioctl(FE_READ_STATUS) error\n");
			return 1;
		}
		if (status & FE_HAS_LOCK) {
			fprintf(stderr, "ok(0x%X)\n", status);
			goto SUCCESS_EXIT;
		}
		nanosleep(&ts, NULL);
	}
	fprintf(stderr, "dvb_lock_check() timeout\n");
	return 1;
SUCCESS_EXIT:;

	// PT2地デジの開始時エラーパケット対策
	// 環境依存の可能性あり
	if (tuner_type == EARTH_PT1) {
		ts.tv_nsec = 200 * 1000 * 1000;  // 200ms
		nanosleep(&ts, NULL);
	}
	return 0;
}

int close_tuner(thread_data *tdata)
{
	struct dtv_property prop[1];
	struct dtv_properties props;
	int rv = 0;

	if (!tdata->table)
		return 0;

	if (tdata->tfd != -1)
		close(tdata->tfd);
	if (tdata->dmxfd > 0)
		close(tdata->dmxfd);
	if (tdata->fefd > 0) {
		if (tdata->table->type == CHTYPE_SATELLITE && tdata->lnb >= 0 && tdata->lnb != SEC_VOLTAGE_OFF) {
			prop[0].cmd = DTV_VOLTAGE;
			prop[0].u.data = SEC_VOLTAGE_OFF;
			props.props = prop;
			props.num = 1;

			if (ioctl(tdata->fefd, FE_SET_PROPERTY, &props) < 0) {
				fprintf(stderr, "LNB OFF failed\n");
				rv = 1;
			}
		}
		close(tdata->fefd);
	}

	tdata->tfd = -1;
	tdata->dmxfd = 0;
	tdata->fefd = 0;
	tdata->table = NULL;

	fprintf(stderr, "tuner closed\n");
	return rv;
}

int lnb_control(int dev_num, int lnb_vol)
{
	struct dtv_property prop[1];
	struct dtv_properties props;
	struct dvb_frontend_info fe_info;
	char device[32];
	int fefd;

	sprintf(device, "/dev/dvb/adapter%d/frontend0", dev_num);
	fefd = open(device, O_RDWR);
	if (fefd < 0) {
		fprintf(stderr, "cannot open \"%s\"\n", device);
		return 1;
	}
	fprintf(stderr, "device = %s\n", device);
	if ((ioctl(fefd, FE_GET_INFO, &fe_info) < 0)) {
		fprintf(stderr, "FE_GET_INFO failed\n");
		close(fefd);
		return 1;
	}
	fprintf(stderr, "Using DVB device \"%s\"\n", fe_info.name);
	if (fe_info.type != FE_QPSK) {
		fprintf(stderr, "tuner is not BS/CS110(FE_QPSK)\n");
		close(fefd);
		return 1;
	}
	prop[0].cmd = DTV_VOLTAGE;
	prop[0].u.data = lnb_vol;
	props.props = prop;
	props.num = 1;
	if (ioctl(fefd, FE_SET_PROPERTY, &props) < 0) {
		fprintf(stderr, "LNB control failed\n");
		close(fefd);
		return 1;
	}
	close(fefd);
	return 0;
}

float getsignal_isdb_s(int signal)
{
	/* apply linear interpolation */
	static const float afLevelTable[] = {
		24.07f,  // 00  00  0      24.07dB
		24.07f,  // 10  00  4096   24.07dB
		18.61f,  // 20  00  8192   18.61dB
		15.21f,  // 30  00  12288  15.21dB
		12.50f,  // 40  00  16384  12.50dB
		10.19f,  // 50  00  20480  10.19dB
		8.140f,  // 60  00  24576  8.140dB
		6.270f,  // 70  00  28672  6.270dB
		4.550f,  // 80  00  32768  4.550dB
		3.730f,  // 88  00  34816  3.730dB
		3.630f,  // 88  FF  35071  3.630dB
		2.940f,  // 90  00  36864  2.940dB
		1.420f,  // A0  00  40960  1.420dB
		0.000f   // B0  00  45056  -0.01dB
	};

	unsigned char sigbuf[4];
	memset(sigbuf, '\0', sizeof(sigbuf));
	sigbuf[0] = (((signal & 0xFF00) >> 8) & 0XFF);
	sigbuf[1] = (signal & 0xFF);

	/* calculate signal level */
	if (sigbuf[0] <= 0x10U) {
		/* clipped maximum */
		return 24.07f;
	} else if (sigbuf[0] >= 0xB0U) {
		/* clipped minimum */
		return 0.0f;
	} else {
		/* linear interpolation */
		const float fMixRate =
			(float)(((unsigned short)(sigbuf[0] & 0x0FU) << 8) |
			        (unsigned short)sigbuf[0]) / 4096.0f;
		return afLevelTable[sigbuf[0] >> 4] * (1.0f - fMixRate) +
		       afLevelTable[(sigbuf[0] >> 4) + 0x01U] * fMixRate;
	}
}

void calc_cn(int fd, int type, boolean use_bell)
{
	int16_t rc;
	int ss_errno, rs_errno;
	double P;
	double CNR;
	int bell = 0;

	if (ioctl(fd, FE_READ_SIGNAL_STRENGTH, &rc) < 0) {
		if (errno != 25) {
			ss_errno = errno;
			if (ioctl(fd, FE_READ_SNR, &rc) < 0) {
				rs_errno = errno;
#ifdef DTV_STAT_SIGNAL_STRENGTH
				struct dtv_property prop[1];
				struct dtv_properties props;

				prop[0].cmd = DTV_STAT_SIGNAL_STRENGTH;
				props.props = prop;
				props.num = 1;

				if (ioctl(fd, FE_GET_PROPERTY, &props) < 0) {
					fprintf(stderr, "ERROR: calc_cn() ioctl(FE_GET_PROPERTY) errno=%d(%s)\n",
					        errno, strerror(errno));
#endif
					fprintf(stderr, "ERROR: calc_cn() ioctl(FE_READ_SIGNAL_STRENGTH) errno=%d(%s)\n",
					        ss_errno, strerror(ss_errno));
					fprintf(stderr, "ERROR: calc_cn() ioctl(FE_READ_SNR) errno=%d(%s)\n",
					        rs_errno, strerror(rs_errno));
					return;
#ifdef DTV_STAT_SIGNAL_STRENGTH
				} else {
					fprintf(stderr, "SNR0: %llu\n", prop[0].u.st.stat[0].uvalue);
					return;
				}
#endif
			} else if (tuner_type & EARTH_PT1)
				CNR = (double)rc / 256;  // 目算なので適当 "* 4 / 1000"かも
			else {
				fprintf(stderr, "SNR: %d\n", rc);
				return;
			}
		} else {
			fprintf(stderr, "ERROR: calc_cn() ioctl(FE_READ_SIGNAL_STRENGTH) errno=%d(%s)\n",
			        errno, strerror(errno));  // Inappropriate ioctl for device
			return;
		}
	} else {
		if (type == CHTYPE_GROUND) {
			P = log10(5505024 / (double)rc) * 10;
			CNR = (0.000024 * P * P * P * P) - (0.0016 * P * P * P) +
			      (0.0398 * P * P) + (0.5491 * P) + 3.0965;
		} else {
			CNR = getsignal_isdb_s(rc);
		}
	}

	if (use_bell) {
		if (CNR >= 30.0)
			bell = 3;
		else if (CNR >= 15.0 && CNR < 30.0)
			bell = 2;
		else if (CNR < 15.0)
			bell = 1;
		fprintf(stderr, "C/N = %fdB (SNR:%d)\n", CNR, rc);
		do_bell(bell);
	} else {
		fprintf(stderr, "C/N = %fdB (SNR:%d)\n", CNR, rc);
	}
	return;
}

void show_channels(void)
{
	fprintf(stderr, "Available Channels:\n");
	fprintf(stderr, "13-62: Terrestrial Channels\n");
	fprintf(stderr, "C13-C63: CATV Channels\n");
	fprintf(stderr, "BS1_0-BS23_1: BS Channels\n");
	fprintf(stderr, "CS2-CS24: CS Channels\n");
	fprintf(stderr, "0x4000-0x7FFF: Satellite Channels(TSID)\n");
}

int parse_time(char *rectimestr, int *recsec)
{
	/* indefinite */
	if (!strcmp("-", rectimestr)) {
		*recsec = -1;
		return 0;
	}
	/* colon */
	else if (strchr(rectimestr, ':')) {
		int n1, n2, n3;
		if (sscanf(rectimestr, "%d:%d:%d", &n1, &n2, &n3) == 3)
			*recsec = n1 * 3600 + n2 * 60 + n3;
		else if (sscanf(rectimestr, "%d:%d", &n1, &n2) == 2)
			*recsec = n1 * 3600 + n2 * 60;
		else
			return 1; /* unsuccessful */

		return 0;
	}
	/* HMS */
	else {
		char *tmpstr;
		char *p1, *p2;
		int flag;

		if (*rectimestr == '-') {
			rectimestr++;
			flag = 1;
		} else
			flag = 0;
		tmpstr = strdup(rectimestr);
		p1 = tmpstr;
		while (*p1 && !isdigit(*p1))
			p1++;

		/* hour */
		if ((p2 = strchr(p1, 'H')) || (p2 = strchr(p1, 'h'))) {
			*p2 = '\0';
			*recsec += (int)strtol(p1, NULL, 10) * 3600;
			p1 = p2 + 1;
			while (*p1 && !isdigit(*p1))
				p1++;
		}

		/* minute */
		if ((p2 = strchr(p1, 'M')) || (p2 = strchr(p1, 'm'))) {
			*p2 = '\0';
			*recsec += (int)strtol(p1, NULL, 10) * 60;
			p1 = p2 + 1;
			while (*p1 && !isdigit(*p1))
				p1++;
		}

		/* second */
		*recsec += (int)strtol(p1, NULL, 10);
		if (flag)
			*recsec *= -1;

		free(tmpstr);

		return 0;
	} /* else */

	return 1; /* unsuccessful */
}

void do_bell(int bell)
{
	int i;
	for (i = 0; i < bell; i++) {
		fprintf(stderr, "\a");
		usleep(400000);
	}
}

int selects(const struct dirent *dir)
{
	if (strncmp(dir->d_name, "adapter", 7)) {
		return 0;
	}
	return 1;
}

void close_dir(struct dirent **namelist, int namecnts)
{
	int i;

	for (i = 0; i < namecnts; i++)
		free(namelist[i]);
	free(namelist);
}

/* from checksignal.c */
int tune(char *channel, thread_data *tdata, int dev_num)
{
	struct dmx_pes_filter_params filter;
	char device[32];
	CHANNEL_SET stock, *table;

	/* get channel */
	if (tdata->table)
		stock = *tdata->table;
	table = searchrecoff(channel);
	if (!table) {
		fprintf(stderr, "Invalid Channel: %s\n", channel);
		return 1;
	}

	/* re-tune */
	if (tdata->table) {
		if (!strcmp(table->parm_freq, stock.parm_freq))
			return 0;
		if (table->type == stock.type) {
			if (set_frequency(tdata, FALSE)) {
				fprintf(stderr, "Cannot tune to the specified channel\n");
				return 1;
			}
			return 0;
		} else {
			tdata->table = &stock;
			if (!close_tuner(tdata) && tdata->queue) {
				/* wait for remainder */
				while (tdata->queue->num_used > 0)
					usleep(10000);
			}
		}
	}
	tdata->table = table;

	/* open tuner */
	/* case 1: specified tuner device */
	if (dev_num >= 0) {
		if (open_tuner(tdata, dev_num, TRUE) != 0) {
			tdata->table = NULL;
			fprintf(stderr, "Cannot open tuner\n");
			return 1;
		}
		while (dvb_lock_check(tdata) != 0) {
			if (tdata->tune_persistent) {
				if (f_exit) {
					close_tuner(tdata);
					return 1;
				}
				fprintf(stderr, "No signal. Still trying: /dev/dvb/adapter%d\n", dev_num);
			} else {
				close_tuner(tdata);
				fprintf(stderr, "Cannot tune to the specified channel: /dev/dvb/adapter%d\n", dev_num);
				return 1;
			}
		}
	} else {
		/* case 2: loop around available devices */
		boolean tuned = FALSE;
#if FULLAUTO_SEARCH
		struct dirent **namelist;
		int lp;
		int num_devs = scandir("/dev/dvb", &namelist, selects, alphasort);

		if (num_devs == -1) {
			perror("scandir");
			tdata->table = NULL;
			return 1;
		}
		for (lp = 0; lp < num_devs; lp++) {
			int count = 0;

			sscanf(namelist[lp]->d_name, "adapter%d", &dev_num);
#else
		int *tuner;
		int num_devs, lp;

		if (tdata->table->type == CHTYPE_SATELLITE) {
			tuner = bsdev;
			num_devs = NUM_BSDEV;
		} else {
			tuner = isdb_t_dev;
			num_devs = NUM_ISDB_T_DEV;
		}

		for (lp = 0; lp < num_devs; lp++) {
			int count = 0;

			dev_num = tuner[lp];
#endif
			if (open_tuner(tdata, dev_num, FALSE) == 0) {
				/* tune to specified channel */
				if (tdata->tune_persistent) {
					while (dvb_lock_check(tdata) != 0 &&
						   count < MAX_RETRY) {
						if (f_exit) {
							close_tuner(tdata);
#if FULLAUTO_SEARCH
							close_dir(namelist, num_devs);
#endif
							return 1;
						}
						fprintf(stderr, "No signal. Still trying: /dev/dvb/adapter%d\n", dev_num);
						count++;
					}

					if (count >= MAX_RETRY) {
						close(tdata->fefd);
						tdata->fefd = 0;
						count = 0;
						continue;
					}
				} /* tune_persistent */
				else {
					if (dvb_lock_check(tdata) != 0) {
						close(tdata->fefd);
						tdata->fefd = 0;
						continue;
					}
				}

				tuned = TRUE;
				if (tdata->tune_persistent)
					fprintf(stderr, "device = /dev/dvb/adapter%d\n", dev_num);
				break; /* found suitable tuner */
			}
#if FULLAUTO_SEARCH
		}
		close_dir(namelist, num_devs);
#else
		}
#endif
		/* all tuners cannot be used */
		if (tuned == FALSE) {
			close_tuner(tdata);
			fprintf(stderr, "Cannot tune to the specified channel\n");
			return 1;
		}
	}

	if (tdata->dmxfd == 0) {
		sprintf(device, "/dev/dvb/adapter%d/demux0", dev_num);
		if ((tdata->dmxfd = open(device, O_RDWR)) < 0) {
			close_tuner(tdata);
			fprintf(stderr, "cannot open demux device\n");
			return 1;
		}
	}

	filter.pid = 0x2000;
	filter.input = DMX_IN_FRONTEND;
	filter.output = DMX_OUT_TS_TAP;
	filter.pes_type = DMX_PES_VIDEO;
	filter.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC;
	if (ioctl(tdata->dmxfd, DMX_SET_PES_FILTER, &filter) == -1) {
		fprintf(stderr, "FILTER %i: ", filter.pid);
		perror("ioctl DMX_SET_PES_FILTER");
		close_tuner(tdata);
		return 1;
	}

	if (tdata->tfd < 0) {
		sprintf(device, "/dev/dvb/adapter%d/dvr0", dev_num);
		if ((tdata->tfd = open(device, O_RDONLY)) < 0) {
			close_tuner(tdata);
			fprintf(stderr, "cannot open dvr device\n");
			return 1;
		}
	}

	if (!tdata->tune_persistent) {
		/* show signal strength */
		calc_cn(tdata->fefd, tdata->table->type, FALSE);
	}

	return 0; /* success */
}
