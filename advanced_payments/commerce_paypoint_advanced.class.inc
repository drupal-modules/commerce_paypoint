<?php

/**
 * @file
 * Class implementing PayPoint's 3rd generation PSP payment service referred as Advanced Payments.
 */

class PPOAdvancedPayment {

  function __construct($url, $username, $password, $hosted = TRUE) {
    $this->url = $url;
    $this->hosted = $hosted;
    $this->username = $username;
    $this->password = $password;
  }

  function send_ping() {
    $this->send_raw_request($this->hosted ? '/hosted/rest/sessions/ping' : '/acceptor/rest/transactions/ping');
  }

  function send_raw_request($uri, $data = array(), $method = 'POST') {

    $headers = array(
      'Accept' => 'application/json',
      'Content-Type'  => 'application/json',
      'Authorization' => "Basic " . base64_encode($this->username . (!empty($this->password) ? ":" . $this->password : '')),
    );

    $result = drupal_http_request($this->url . $uri, array(
      'headers' => $headers,
      'method' => $method,
      'data' => http_build_query($data)
    ));
    return $result;
  }

  function PaymentRequest() {
  }

  // readRawFile:
  // MakePaymentRequestTest:
  // validateCredentials:
  // validateAmount:
  // validateBaseURL:
  // validateCard:
  // validateCardCVV:
  // validateTransaction:
  // initWithBaseURL:
  /*
     com.paypoint.sdk.library.exception.InvalidCredentialsException
     com.paypoint.sdk.library.exception.PaymentValidationException
     com.paypoint.sdk.library.exception.TransactionInProgressException
     com.paypoint.sdk.library.exception.TransactionSuspendedFor3DSException

     com.paypoint.sdk.library.exception.PaymentValidationException.ErrorCode
     com.paypoint.sdk.library.network.EndpointManager.Environment
     com.paypoint.sdk.library.payment.PaymentError.ReasonCode
  */
}
