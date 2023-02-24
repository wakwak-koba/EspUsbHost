#include "EspUsbHost.h"
#include "unicodeConverter.h"

void EspUsbHost::begin(void) {
  const usb_host_config_t config = {
    .skip_phy_setup = false,    
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  esp_err_t err = usb_host_install(&config);
  if (err != ESP_OK) {
    ESP_LOGI("EspUsbHost", "usb_host_install err=%x", err);
  }

  const usb_host_client_config_t client_config = {
    .is_synchronous = true,
    .max_num_event_msg = 10,
    .async = {
      .client_event_callback = this->_clientEventCallback,
      .callback_arg = this,
    }
  };
  err = usb_host_client_register(&client_config, &this->clientHandle);
  if (err != ESP_OK) {
    ESP_LOGI("EspUsbHost", "usb_host_client_register() err=%x", err);
  }
}

void EspUsbHost::_clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg) {
  EspUsbHost *usbHost = (EspUsbHost *)arg;

  esp_err_t err;
  switch (eventMsg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      ESP_LOGI("EspUsbHost", "USB_HOST_CLIENT_EVENT_NEW_DEV new_dev.address=%d", eventMsg->new_dev.address);
      err = usb_host_device_open(usbHost->clientHandle, eventMsg->new_dev.address, &usbHost->deviceHandle);
      if (err != ESP_OK) {
        ESP_LOGI("EspUsbHost", "usb_host_device_open() err=%x", err);
      }

      err = usb_host_device_info(usbHost->deviceHandle, &usbHost->deviceInfo);
      if (err != ESP_OK) {
        ESP_LOGI("EspUsbHost", "usb_host_device_info() err=%x", err);
      }
      usbHost->_configCallback(&usbHost->deviceInfo);

      const usb_device_desc_t *dev_desc;
      err = usb_host_get_device_descriptor(usbHost->deviceHandle, &dev_desc);
      if (err != ESP_OK) {
        ESP_LOGI("EspUsbHost", "usb_host_get_device_descriptor() err=%x", err);
      }
      usbHost->deviceDesc = *dev_desc;
      usbHost->_configCallback(&usbHost->deviceDesc);

      const usb_config_desc_t *config_desc;
      err = usb_host_get_active_config_descriptor(usbHost->deviceHandle, &config_desc);
      if (err != ESP_OK) {
        ESP_LOGI("EspUsbHost", "usb_host_get_active_config_descriptor() err=%x", err);
      }
      usbHost->_configCallback(config_desc);

      break;
    
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      ESP_LOGI("EspUsbHost", "USB_HOST_CLIENT_EVENT_DEV_GONE dev_gone.dev_hdl=%x", eventMsg->dev_gone.dev_hdl);
      usbHost->onGone(&eventMsg->dev_gone.dev_hdl);
      usbHost->onGone();
      usb_host_device_close(usbHost->clientHandle, usbHost->deviceHandle);
      usbHost->raisedOnNew = false;
      break;

    default:
      ESP_LOGI("EspUsbHost", "clientEventCallback() default %d", eventMsg->event);
      break;
  }
}

void EspUsbHost::_configCallback(const usb_device_info_t *dev_info) {
  this->onConfig(dev_info);
}

void EspUsbHost::_configCallback(const usb_device_desc_t *dev_desc) {
  this->onConfig(dev_desc);
}

void EspUsbHost::_configCallback(const usb_config_desc_t *config_desc) {
  ESP_LOGI("", "bLength: %d", config_desc->bLength);
  ESP_LOGI("", "bDescriptorType(config): %d", config_desc->bDescriptorType);
  ESP_LOGI("", "wTotalLength: %d", config_desc->wTotalLength);
  ESP_LOGI("", "bNumInterfaces: %d", config_desc->bNumInterfaces);
  ESP_LOGI("", "bConfigurationValue: %d", config_desc->bConfigurationValue);
  ESP_LOGI("", "iConfiguration: %d", config_desc->iConfiguration);
  ESP_LOGI("", "bmAttributes(%s%s%s): 0x%02x",
      (config_desc->bmAttributes & USB_BM_ATTRIBUTES_SELFPOWER)?"Self Powered":"",
      (config_desc->bmAttributes & USB_BM_ATTRIBUTES_WAKEUP)?", Remote Wakeup":"",
      (config_desc->bmAttributes & USB_BM_ATTRIBUTES_BATTERY)?", Battery Powered":"",
      config_desc->bmAttributes);
  ESP_LOGI("", "bMaxPower: %d = %d mA", config_desc->bMaxPower, config_desc->bMaxPower * 2);    

  const uint8_t *p = &config_desc->val[0];
  uint8_t bLength;
  for (int i = 0; i < config_desc->wTotalLength; i += bLength, p += bLength) {
    bLength = *p;
    if ((i + bLength) <= config_desc->wTotalLength) {
      const uint8_t bDescriptorType = *(p + 1);
      this->onConfig(bDescriptorType, p);
    } else {
      break;
    }
  }
  
  this->onConfig(config_desc);
}

void EspUsbHost::task(void) {
  if(!this->raisedOnNew && this->isReady()) {
    this->onNew();
    this->raisedOnNew = true;
  }

  esp_err_t err = usb_host_lib_handle_events(1, &this->eventFlags);
  if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
    ESP_LOGI("EspUsbHost", "usb_host_lib_handle_events() err=%x eventFlags=%x", err, this->eventFlags);
  }

  err = usb_host_client_handle_events(this->clientHandle, 1);
  if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
    ESP_LOGI("EspUsbHost", "usb_host_client_handle_events() err=%x", err);
  }
}

std::string EspUsbHost::getManufacturer() {
  std::string result;
  if(deviceInfo.str_desc_manufacturer->bLength > 4)
    result = utf::toString((char16_t *)deviceInfo.str_desc_manufacturer->wData, deviceInfo.str_desc_manufacturer->bLength / 2 - 1);
  return result;
}
std::string EspUsbHost::getProduct() {
  std::string result;
  if(deviceInfo.str_desc_product     ->bLength > 4)
    result = utf::toString((char16_t *)deviceInfo.str_desc_product     ->wData, deviceInfo.str_desc_product     ->bLength / 2 - 1);
  return result;
}
std::string EspUsbHost::getSerialNum() {
  std::string result;
  if(deviceInfo.str_desc_serial_num  ->bLength > 4)
    result = utf::toString((char16_t *)deviceInfo.str_desc_serial_num  ->wData, deviceInfo.str_desc_serial_num  ->bLength / 2 - 1);
  return result;
}

esp_err_t EspUsbHost::submit_control(const uint8_t requestType, const uint8_t bRequest, const uint16_t wValue) {
  return submit_control(requestType, bRequest, wValue, 0x0000, 0x0000, nullptr);
}
esp_err_t EspUsbHost::submit_control(const uint8_t requestType, const uint8_t bRequest, const uint16_t wValue, const uint16_t wIndex, const uint16_t wLength, const void *data) {
  usb_transfer_t *usbTransfer_cmd;
  usb_host_transfer_alloc(8 + wLength, 0, &usbTransfer_cmd);

  usbTransfer_cmd->num_bytes = 8;
  usbTransfer_cmd->data_buffer[0] = requestType;
  usbTransfer_cmd->data_buffer[1] = bRequest;
  usbTransfer_cmd->data_buffer[2] = wValue & 0xff;
  usbTransfer_cmd->data_buffer[3] = wValue >> 8;
  usbTransfer_cmd->data_buffer[4] = wIndex & 0xff;
  usbTransfer_cmd->data_buffer[5] = wIndex >> 8;
  usbTransfer_cmd->data_buffer[6] = wLength & 0xff;
  usbTransfer_cmd->data_buffer[7] = wLength >> 8;
  if(wLength > 0 && data)
    memcpy(&usbTransfer_cmd->data_buffer[8], data, wLength);
  
  usbTransfer_cmd->device_handle = deviceHandle;
  usbTransfer_cmd->bEndpointAddress = 0x00;
    usbTransfer_cmd->callback = [](usb_transfer_t *transfer) {usb_host_transfer_free(transfer);};
  usbTransfer_cmd->context = this;

  return usb_host_transfer_submit_control(clientHandle, usbTransfer_cmd);
}
