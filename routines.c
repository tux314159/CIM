/* To avoid the huge if-else tree; though this isn't much better :P */
goto END_ROUTINES;
SEND:;;
     recv(clifd, buf, BUFMAX - 1, 0);
     strcpy(stat->msgstack[stat->stacksz++], buf);
     send(clifd, "RECV", 4, 0);
     goto END_ROUTINES;

GETN:;;
     recv(clifd, buf, sizeof(buf), 0);
     temp = atoi(buf);
     if (temp >= stat->stacksz)
	     write(clifd, "INDO", 4); /* INDex Over */
     else if (temp < 0)
	     write(clifd, "INDU", 4); /* INDex Under */
     else {
	     strcat(buf, "RMSG"); /* Return MeSsaGe */
	     strcat(buf, stat->msgstack[temp]);
	     send(clifd, buf + 1, sizeof(buf) - 1, 0);
     }
     goto END_ROUTINES;

GETL:;;
     if (stat->stacksz == 0)
	     write(clifd, "INDO", 4);
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

TERM:;;
     printf("SERVER: CHILD: Got TERM!\n");
     stat->term = true;
     send(clifd, "TERM", 4, 0);
     goto END_ROUTINES;

WHAT:;;
     send(clifd, "WHAT", 4, 0);
     goto END_ROUTINES;

END_ROUTINES:;;
