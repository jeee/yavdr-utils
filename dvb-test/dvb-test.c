#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>
#include <sys/ioctl.h>

struct tDvbParameterMap {
	int userValue;
	int driverValue;
	const char *userString;
};

static int wait = 0;
static int syslog = 0;
static int quiet = 0;

const struct tDvbParameterMap ModulationValues[] = { { 16, QAM_16, "QAM16" }, {
		32, QAM_32, "QAM32" }, { 64, QAM_64, "QAM64" }, { 128, QAM_128,
		"QAM128" }, { 256, QAM_256, "QAM256" }, { 2, QPSK, "QPSK" }, { 5,
		PSK_8, "8PSK" }, { 6, APSK_16, "16APSK" }, { 10, VSB_8, "VSB8" }, { 11,
		VSB_16, "VSB16" }, { 998, QAM_AUTO, "QAMAUTO" }, { -1, 0, NULL } };

const char *DeliverySystems[] = { "UNDEFINED", "DVB-C", "DVB-C", "DVB-T",
		"DSS", "DVB-S", "DVB-S2", "DVB-H", "ISDBT", "ISDBS", "ISDBC", "ATSC",
		"ATSCMH", "DMBTH", "CMMB", "DAB", NULL };

int DriverIndex(int Value, const struct tDvbParameterMap *Map) {
	const struct tDvbParameterMap *map = Map;
	while (map && map->userValue != -1) {
		if (map->driverValue == Value)
			return map - Map;
		map++;
	}
	return -1;
}

const char *MapToUserString(int Value, const struct tDvbParameterMap *Map) {
	int n = DriverIndex(Value, Map);
	if (n >= 0)
		return Map[n].userString;
	return "???";
}
/*
 int test() {
 cString FileName = DvbName(DEV_DVB_FRONTEND, Adapter, Frontend);
 if (access(FileName, F_OK) == 0) {
 int f = open(FileName, O_RDONLY);
 if (f >= 0) {
 close(f);
 return true;
 } else if (errno != ENODEV && errno != EINVAL) {
 LOG_ERROR_STR("open");
 LOG_ERROR_STR(*FileName);
 }
 } else if (errno != ENOENT) {
 LOG_ERROR_STR("access");
 LOG_ERROR_STR(*FileName);
 }
 return false;
 }
 */

int main(int argc, char *argv[]) {
	int ret = 0;
	int c;
	int i;
	int numProvidedSystems = 0;
	struct dvb_frontend_info frontendInfo;
	enum fe_delivery_system frontendType = SYS_UNDEFINED;

	while (1) {
		int longindex = 0;

		static struct option longopts[] = { { "wait", 0, &wait, 1 }, { "quiet",
				0, &quiet, 1 }, { "syslog", 0, &syslog, 1 }, { 0, 0, 0, 0 } };

		if ((c = getopt_long(argc, argv, "", longopts, &longindex)) == -1)
			break;

	}
	if (optind < argc) {
		for (i = optind; i < argc; i++) {
			if (quiet == 0)
				printf("checking : %s\n", argv[i]);

			do {
				if (access(argv[i], F_OK) == 0) {
					int fd_frontend = open(argv[i], O_RDONLY);

					if (fd_frontend >= 0) {
						if (ioctl(fd_frontend, FE_GET_INFO, &frontendInfo) >= 0) {
							switch (frontendInfo.type) {
							case FE_QPSK:
								frontendType = (frontendInfo.caps
										& FE_CAN_2G_MODULATION) ? SYS_DVBS2
										: SYS_DVBS;
								break;
							case FE_OFDM:
								frontendType = SYS_DVBT;
								break;
							case FE_QAM:
								frontendType = SYS_DVBC_ANNEX_AC;
								break;
							case FE_ATSC:
								frontendType = SYS_ATSC;
								break;
							default:
								fprintf(
										stderr,
										"ERROR: unknown frontend type %d on frontend %s",
										frontendInfo.type, argv[i]);
							}
							if (frontendType != SYS_UNDEFINED) {
								numProvidedSystems++;
								if (frontendType == SYS_DVBS2)
									numProvidedSystems++;
								char Modulations[64];
								char *p = Modulations;
								if (frontendInfo.caps & FE_CAN_QPSK) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QPSK,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_QAM_16) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QAM_16,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_QAM_32) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QAM_32,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_QAM_64) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QAM_64,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_QAM_128) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QAM_128,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_QAM_256) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(QAM_256,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_8VSB) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(VSB_8,
													ModulationValues));
								}
								if (frontendInfo.caps & FE_CAN_16VSB) {
									numProvidedSystems++;
									p += sprintf(
											p,
											",%s",
											MapToUserString(VSB_16,
													ModulationValues));
								}
								//if (frontendInfo.caps & FE_CAN_TURBO_FEC){numProvidedSystems++; p += sprintf(p, ",%s", "TURBO_FEC"); }
								if (p != Modulations)
									p = Modulations + 1; // skips first ','
								else {
									p = (char *) "unknown modulations";
								}

								if (quiet == 0)
									printf(
											"frontend %s provides %s with %s (\"%s\")\n",
											argv[i],
											DeliverySystems[frontendType], p,
											frontendInfo.name);
								else
									printf("%s\n",
											DeliverySystems[frontendType]);
							}
						}
						if (quiet == 0) printf("frontend %s is ready\n", argv[i]);
					} else {
						if (quiet == 0) printf("frontend %s not ready\n", argv[i]);
					}
				} else {
					fprintf(stderr, "access to frontend %s denied\n", argv[i]);
				}
			} while (wait == 1);
		}
	} else {
		fprintf(stderr, "no frontend is given!\n");
	}

	return ret;
}
