/**
 * @file email_module.h
 * @author Yassine Hattay (hattayyassine519@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-02
 *
 * 
 *
 */

#ifndef SMTP_H
#define SMTP_H

#include <ESP_Mail_Client.h>

#define emailSenderAccount "yassinebeebotte@gmail.com"
/** @cond */
#define emailSenderPassword ""
/** @endcond */
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
