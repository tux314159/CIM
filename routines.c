/* To avoid the huge if-else tree; though this isn't much better :P */
sem_wait(&stat->sem);
goto END_ROUTINES;
SEND:;;
	 recv(clifd, buf, BUFMAX - 1, 0);
	 strcpy(stat->msgstack[stat->stacksz++], buf);
	 sprintf(buf, "RECV%06lu", stat->stacksz); /* Return MeSsaGe */
	 send(clifd, buf, sizeof(buf), 0);
	 goto END_ROUTINES;

GETN:;;
	 recv(clifd, buf, sizeof(buf), 0);
	 temp = atoi(buf) - 1;
	 if (temp >= stat->stacksz)
		 send(clifd, "INDO", 4, 0); /* INDex Over */
	 else if (temp < 0)
		 send(clifd, "INDU", 4, 0); /* INDex Under */
	 else {
		 sprintf(buf, "RMSG%06lu%s", stat->stacksz, stat->msgstack[temp]); /* Return MeSsaGe */
		 send(clifd, buf + 1, sizeof(buf) - 1, 0);
	 }
	 goto END_ROUTINES;

GETL:;;
	 if (stat->stacksz == 0)
		 send(clifd, "INDO", 4, 0);
	 else {
		 strcat(buf, "RMSG");
		 strcat(buf, stat->msgstack[stat->stacksz - 1]);
		 send(clifd, buf, sizeof(buf), 0);
	 }
	 goto END_ROUTINES;

GSSZ:;;
	 char t[4 + 4];
	 sprintf(t, "RSSZ%lu", stat->stacksz); /* Return Stack SiZe */
	 send(clifd, t, sizeof(t), 0);
	 goto END_ROUTINES;

SKIP:;;
	send(clifd, "    ", 4, 0); /* Client is expecting 4 bytes */
	goto END_ROUTINES;

WHAT:;;
	 send(clifd, "WHAT", 4, 0);
	 goto END_ROUTINES;

END_ROUTINES:;;
sleep(0.001);
sem_post(&stat->sem);
