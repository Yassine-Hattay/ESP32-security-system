#include "email_module.h"
#include "global_header.h"

SMTPSession smtp;
 
void smtpCallback(SMTP_Status status)
{
  Serial.println(status.info());

  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t a = 0; a < smtp.sendingResult.size(); a++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(a);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", a + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}


void sendPhoto(void)
{

  smtp.debug(1);

  smtp.callback(smtpCallback);

  Session_Config config;

  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 0;
  config.time.day_light_offset = 1;

  config.server.host_name = smtpServer;
  config.server.port = smtpServerPort;
  config.login.email = emailSenderAccount;
  config.login.password = emailSenderPassword;
  config.login.user_domain = "";

  SMTP_Message message;

  message.enable.chunking = true;

  message.sender.name = "ESP32-CAM";
  message.sender.email = emailSenderAccount;

  message.subject = emailSubject;
  message.addRecipient("yassine", emailRecipient1);
  message.addRecipient("yassine", emailRecipient2);

  String htmlMsg = "<h2>Photo captured with ESP32-CAM and attached in this email.</h2>";
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_qp;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  SMTP_Attachment att;

  att.descr.filename = FILE_PHOTO;
  att.descr.mime = "image/png";
  att.file.path = FILE_PHOTO_PATH;
  att.file.storage_type = esp_mail_file_storage_type_flash;
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  message.addAttachment(att);

  if (!smtp.connect(&config))
  {
    connected_internet = false;
  } //  mdse
  if (!MailClient.sendMail(&smtp, &message, false))
  {
    connected_internet = false;
  } //  mdse
}

bool checkSMTPService(const char *server, uint16_t port, const char *email, const char *password)
{
  Session_Config sessionConfig;
  sessionConfig.server.host_name = server;
  sessionConfig.server.port = port;
  sessionConfig.login.email = email;
  sessionConfig.login.password = password;

  // Attempt to connect to the SMTP server
  Serial.printf("Checking SMTP server %s on port %d...\n", server, port);
  if (smtp.connect(&sessionConfig, false))
  { // 'false' means don't log in
    Serial.println("SMTP server is available!");
    smtp.closeSession(); // Close the connection
    return true;
  }
  else
  {
    Serial.printf("Failed to connect to SMTP server. Error: %s\n", smtp.errorReason().c_str());
    return false;
  }
}





