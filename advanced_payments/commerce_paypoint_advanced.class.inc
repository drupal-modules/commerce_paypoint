<?php

/**
 * @file
 * Class implementing PayPoint's 3rd generation PSP payment service referred as Advanced Payments.
 */

class PPOAdvancedPayment {

  private $url = PAYPOINT_ADV_MITE_HOSTED_ENDPOINT;
  private $instId;
  private $hosted = TRUE;
  private $username, $password;
  private $response; // Last response.

  // Default request objects.
  private $session = array(
            'returnUrl' => array(
              'url' => 'http://example.com',
            ),
            'transactionNotification' => array (
              'url' => 'http://www.example.com',
              'format' => 'REST_XML',
            ),
          );
  private $transaction = array(
            'merchantReference' => 'your_reference_here',
            'money' => array (
              'currency' => 'GBP',
              'amount' => array (
                'fixed' => '1.00',
              ),
            ),
            'description' => ' Muneebs Test Transaction',
          );
  private $customer = array(
            'create' => true,
            'identity' => array (
              'merchantCustomerId' => '1111111111112',
            ),
            'details' => array (
              'name' => 'Mr Foo Bar',
              'address' => array (
                'line1' => 'Address Line 1',
                'line2' => 'Address Line 2',
                'city' => 'City',
                'region' => 'Region',
                'postcode' => 'AVS111',
                'countryCode' => 'GBR',
              ),
              'telephone' => '0044111111111',
              'emailAddress' => 'example@example.com',
              'ipAddress' => '127.0.0.1',
              'defaultCurrency' => 'GBP',
            ),
          );

  function __construct($url, $username, $password, $instId, $hosted = TRUE) {
    $this->url = $url ?: PAYPOINT_ADV_MITE_HOSTED_ENDPOINT;
    $this->hosted = $hosted ?: TRUE;
    $this->username = $username;
    $this->password = $password;
    $this->instId = (int)$instId;
    $this->setSession();
  }

  /*
   * Test connectivity to Intelligent Commerce using the Ping request.
   */
  function sendPing() {
    return $this->sendRawRequest(
        $this->hosted ? '/hosted/rest/sessions/ping' : '/acceptor/rest/transactions/ping',
        array(),
        'GET');
  }

  /*
   * Make a payment.
   *
   * Submit customer payments initiated online or over the phone using the Payment Request
   * for a one-step authorisation and capturing of funds. The request can additionally
   * be processed with 3D Secure. Configuring your account for 3D Secure can be performed
   * at boarding or at a later date – it’s up to you.
   *
   * @see: https://developer.paypoint.com/payments/docs/#payments/make_a_payment
   */
  function sendProcessPayment() {

    $request                = array();
    $request['session']     = $this->session;
    $request['transaction'] = $this->transaction;
    $request['customer']    = $this->customer;

    // print_r(($request)); exit;

    return $this->sendRawRequest(
      $this->hosted ? "/hosted/rest/sessions/$this->instId/payments" : "/acceptor/rest/transactions/$this->instId/payment",
      $request
    );
  }

  function sendRawRequest($uri, $data, $method = 'POST') {

    $headers = array(
      'Accept' => 'application/json',
      'Content-Type'  => 'application/json',
      'Authorization' => "Basic " . base64_encode($this->username . (!empty($this->password) ? ":" . $this->password : '')),
    );

    $response = drupal_http_request($this->url . $uri, array(
      'headers' => $headers,
      'method' => $method,
      'data' => is_array($data) ? json_encode($data) : $data,
    ));
    if ($response->code == 201) {
      unset($response->error); # Workaround for Drupal bug: https://www.drupal.org/node/2350033
    }
    $this->response = $response;
    if (!empty($response->error)) {
      throw new Exception($response->error, $response->code);
    }
    return $response;
  }

  // Getters
  function getLastData() {
    return $this->response ? json_decode($this->response->data) : FALSE;
  }

  function getLastCode() {
    return $this->response ? $this->response->code : FALSE;
  }

  function getLastStatus() {
    return $this->response ? json_decode($this->response->data)->status : "UNKNOWN";
  }

  function getLastResponse() {
    return $this->response ? $this->response : FALSE;
  }

  function paymentRequest() {
  }

  // Setters //

  /*
   * Set PayPoint hosted session.
   *
   *  @param
   *    $returnUrl string Mandatory
   *      Mandatory: The URL that we will return your customer after the processing the transaction.
   *    $cancelUrl string Mandatory
   *      The URL that we will return your customer to if they cancel the hosted session.
   *      If omitted the returnUrl is used if they cancel.
   *    $restoreUrl string Mandatory
   *      The URL of PayPoint hosted session.
   *      We'll take your customer to this URL in the event they cancel their PayPal payment
   *      with the "Cancel and return to...." link. You will need to use this field if you iFrame our hosted product.
   *    $notificationUrl string Mandatory
   *      The URL you want the callback or notification to be sent to. This will override any defaults set on your account. Where a default is set and a blank URL field is specified, no callback or notification will be sent.
   *    $notificationFormat string
   *      Possible Values: REST_XML, REST_JSON
   *      The format of the callback content.
   *    $features string
   *      Possible Values: always, optional
   *      Allow the customer to choose if they wish their payment method to be registered.
   *    $skin string
   *      The ID of the skin used to drive look and feed for this session. Refer to Customise hosted look and feel for more information.
   */
  function setSession(
        $returnUrl  = 'http://example.com',
        $cancelUrl,
        $restoreUrl,
        $preAuthCallbackUrl,
        $preAuthCallbackFormat,
        $postAuthCallbackUrl,
        $postAuthCallbackFormat,
        $notificationUrl,
        $notificationFormat,
        $features,                // @todo
        $skin                     // @todo
      ) {
    $this->session = array();
    !empty($returnUrl)  && $this->session['returnUrl']['url']  = $returnUrl;
    !empty($cancelUrl)  && $this->session['cancelUrl']['url']  = $cancelUrl;
    !empty($restoreUrl) && $this->session['restoreUrl']['url'] = $returnUrl;

    !empty($preAuthCallbackUrl)     && $this->session['transactionpreAuthCallback']['url'] = $preAuthCallbackUrl;
    !empty($preAuthCallbackFormat)  && $this->session['transactionpreAuthCallback']['format'] = $preAuthCallbackFormat;
    !empty($postAuthCallbackUrl)    && $this->session['transactionpostAuthCallback']['url'] = $postAuthCallbackUrl;
    !empty($postAuthCallbackFormat) && $this->session['transactionpostAuthCallback']['format'] = $postAuthCallbackFormat;
    !empty($notificationUrl)        && $this->session['transactionNotification']['url'] = $notificationUrl;
    !empty($notificationFormat)     && $this->session['transactionNotification']['format'] = $notificationFormat;
  }

  /*
   * Test connectivity to Intelligent Commerce using the Ping request.
   */
  function setTransaction() {
    $this->transaction = array(
      'merchantReference' => 'your_reference_here',
      'money' => array (
        'currency' => 'GBP',
        'amount' => array (
          'fixed' => '1.00',
        ),
      ),
      'description' => ' Muneebs Test Transaction',
    );
  }

  /*
   * Test connectivity to Intelligent Commerce using the Ping request.
   */
  function setCustomer() {
    $customer = array(
      'create' => true,
      'identity' => array (
        'merchantCustomerId' => '1111111111112',
      ),
      'details' => array (
        'name' => 'Mr Foo Bar',
        'address' => array (
          'line1' => 'Address Line 1',
          'line2' => 'Address Line 2',
          'city' => 'City',
          'region' => 'Region',
          'postcode' => 'AVS111',
          'countryCode' => 'GBR',
        ),
        'telephone' => '0044111111111',
        'emailAddress' => 'example@example.com',
        'ipAddress' => '127.0.0.1',
        'defaultCurrency' => 'GBP',
      ),
    );
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
