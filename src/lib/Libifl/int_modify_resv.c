/*
 * Copyright (C) 1994-2019 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * For a copy of the commercial license terms and conditions,
 * go to: (http://www.pbspro.com/UserArea/agreement.html)
 * or contact the Altair Legal Department.
 *
 * Altair’s dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of PBS Pro and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair’s trademarks, including but not limited to "PBS™",
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
 * trademark licensing policies.
 *
 */

#include <pbs_config.h>   /* the master config generated by configure */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "portability.h"
#include "libpbs.h"
#include "dis.h"
#include "rpp.h"

/**
 * @brief Sends the Modify Reservation request
 *
 * @param[in] connect - socket descriptor for the connection.
 * @param[in] resv-Id - Reservation Identifier
 * @param[in] attrib  - list of attributes to be modified.
 * @param[in] extend  - extended options
 *
 * @return - reply from server on no error.
 * @return - NULL on error.
 */

char *
PBSD_modify_resv(int connect, char *resv_id, struct attropl *attrib, char *extend)
{
	struct batch_reply	*reply = NULL;
	int			rc = -1;
	char			*ret = NULL;
	int			sock = connection[connect].ch_socket;

	DIS_tcp_setup(sock);

	/* first, set up the body of the Modify Reservation request */

	if ((rc = encode_DIS_ReqHdr(sock, PBS_BATCH_ModifyResv, pbs_current_user)) ||
		(rc = encode_DIS_ModifyResv(sock, resv_id, attrib)) ||
		(rc = encode_DIS_ReqExtend(sock, extend))) {
			connection[connect].ch_errtxt = strdup(dis_emsg[rc]);
			if (connection[connect].ch_errtxt == NULL)
				return NULL;
			if (pbs_errno == PBSE_PROTOCOL)
				return NULL;
	}
	if (DIS_tcp_wflush(sock)) {
		pbs_errno = PBSE_PROTOCOL;
		return NULL;
	}

	reply = PBSD_rdrpy(connect);
	if (reply == NULL)
		pbs_errno = PBSE_PROTOCOL;
	else {
		if ((reply->brp_code == PBSE_NONE) && (reply->brp_un.brp_txt.brp_str)) {
			ret = strdup(reply->brp_un.brp_txt.brp_str);
			if (!ret)
				pbs_errno = PBSE_SYSTEM;
		}
		PBSD_FreeReply(reply);
	}
	return ret;
}

