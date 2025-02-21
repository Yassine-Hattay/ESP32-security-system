#ifndef SMTP_H
#define SMTP_H

#include <ESP_Mail_Client.h>
// Email credentials and settings
#define emailSenderAccount "yassinebeebotte@gmail.com"
#define emailSenderPassword "gsti ffmp djsg kfhf"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "Mouvement détecté"
#define emailRecipient1 "hattayyassine519@gmail.com"
#define emailRecipient2 "yassine.hattay@etudiant-fst.utm.tn"

// Declare global smtp object and function prototypes
extern SMTPSession smtp;

void smtpCallback(SMTP_Status status);
void sendPhoto(void);
bool checkSMTPService(const char *server, uint16_t port, const char *email, const char *password);

#endif
