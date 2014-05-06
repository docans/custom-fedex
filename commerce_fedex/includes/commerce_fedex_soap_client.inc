<?php

/**
 * @file
 * Handles the SOAP request/response to FedEx Web Services servers.
 */

/**
 * Function to create FedEx rate request array.
 *
 * @param object $order
 *   The order object.
 *
 * @return array
 *   The array that is created for getting rates from FedEx.
 */
function commerce_fedex_create_rate_request($order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

  $request['TransactionDetail'] = array('CustomerTransactionId' => ' *** Rate Request v13 using Drupal Commerce ***');
  $request['Version'] = array(
    'ServiceId' => 'crs',
    'Major' => '13',
    'Intermediate' => '0',
    'Minor' => '0',
  );

  // Load the number of packages and their physical attributes.
  $package_line_items = _commerce_fedex_get_package_items($order, $order_wrapper);

  // Make sure that there are packages to be sent in the request.
  if (!empty($package_line_items)) {
    $request['ReturnTransitAndCommit'] = TRUE;
    $request['RequestedShipment']['DropoffType'] = variable_get('commerce_fedex_dropoff', 'REGULAR_PICKUP');
    $request['RequestedShipment']['ShipTimestamp'] = date('c');
    $request['RequestedShipment']['PackagingType'] = variable_get('commerce_fedex_default_package_type', 'YOUR_PACKAGING');
    $request['RequestedShipment']['Shipper'] = _commerce_fedex_get_shipper();
    $request['RequestedShipment']['Recipient'] = _commerce_fedex_get_recipient($order, $order_wrapper);
    $request['RequestedShipment']['RateRequestTypes'] = strtoupper(variable_get('commerce_fedex_rate_service_type', 'list'));
    $request['RequestedShipment']['PackageDetail'] = 'INDIVIDUAL_PACKAGES';
    $request['RequestedShipment']['PackageCount'] = count($package_line_items);
    $request['RequestedShipment']['RequestedPackageLineItems'] = $package_line_items;

    // Return the request.
    return $request;
  }
  else {
    // If there are no shippable packages in the order.
    return FALSE;
  }
}

/**
 * Submits a SOAP request to FedEx and returns the response object.
 *
 * @param object $request
 *   The order object.
 * @param string $method
 *   The method to invoke when submitting the request.
 * @return object
 *   The response object returned after submitting the SOAP request.
 */
function commerce_fedex_submit_soap_request($request, $method = 'getRates') {
  // Add the authentication array to the request.
  $request += _commerce_fedex_soap_authentication();

  // Allow other modles the ability to alter the request before sending.
  drupal_alter('commerce_fedex_submit_soap_request', $request, $method);

  // Log the API request if specified.
  if (in_array('request', variable_get('commerce_fedex_log', array()))) {
    $message = t('Submitting API request to the FedEx');
    watchdog('fedex', '@message:<pre>@request</pre>', array('@message' => $message, '@request' => print_r($request, TRUE)));
  }

  ini_set('soap.wsdl_cache_enabled', '0');

  // Determine if the production or testing WSDL should be used.
  $request_mode = variable_get('commerce_fedex_request_mode', 'testing');

  // Determine the correct wsdl file to use for this method.
  $wsdl_file = _commerce_fedex_soap_wsdl_file($method);

  // Locate the WDSL file for describing the web service.
  $wsdl = drupal_get_path('module', 'commerce_fedex') . '/includes/wsdl-' . $request_mode . '/' . $wsdl_file;

  // Make sure that the wsdl file exists before attempting the request.
  if(!file_exists($wsdl)) {
    return FALSE;
  }

  try {
    // Create the SOAP client.
    $client = new SoapClient($wsdl, array('trace' => 1));
    // Attempt the SOAP request.
    $response = $client->$method($request);
    // Log the API request if specified.
    if (in_array('response', variable_get('commerce_fedex_log', array()))) {
      watchdog('fedex', 'API response received:<pre>@response</pre>', array('@response' => print_r($response, TRUE)));
    }

    $errors = array('FAILURE', 'ERROR');
    if (!in_array($response->HighestSeverity, $errors)) {
      // Return the response object if there are no fatal errors.
      return $response;
    }
  } catch (SoapFault $exception) {
    watchdog('fedex', '<h2>SOAP Fault</h2><br /><b>Code:</b> @faultcode <br /><b>String:</b> @faultstring', array('@faultcode' => $exception->faultcode, '@faultstring' => $exception->faultstring));
  }
  return FALSE;
}

/**
 * Internal function to build authentication array for SOAP request.
 *
 * @return array
 *   An array including all of the authentication values for FedEx.
 */
function _commerce_fedex_soap_authentication() {
  $authentication = array();

  // Add the FedEx web services key and password credentials.
  $authentication['WebAuthenticationDetail'] = array(
    'UserCredential' => array(
      'Key' => variable_get('commerce_fedex_key', NULL),
      'Password' => variable_get('commerce_fedex_password', NULL),
    ),
  );

  // Add the FedEx web services account and meter numbers.
  $authentication['ClientDetail'] = array(
    'AccountNumber' => variable_get('commerce_fedex_account_number', NULL),
    'MeterNumber' => variable_get('commerce_fedex_meter_number', NULL),
  );

  return $authentication;
}

/**
 * Internal function to return an array of wsdl files keyed by their metho
 *
 * @param string $method
 *   The FedEx API method being called.
 * @return string
 *   The file name that corresponds with the method being called.
 */
function _commerce_fedex_soap_wsdl_file($method) {
  // Build an array of wsdl file names that is keyed by FedEx API methods.
  $files = array(
    'getRates' => 'RateService_v13.wsdl',
  );
  if (!empty($files[$method])) {
    return $files[$method];
  }
  return FALSE;
}

/**
 * Internal function to build shipper (ship from) array.
 *
 * @return array
 *   An array that represents the ship from address.
 */
function _commerce_fedex_get_shipper() {
  $shipper = array(
    'Contact' => array(
      'PersonName' => variable_get('commerce_fedex_company_name'),
      'CompanyName' => variable_get('commerce_fedex_company_name'),
    ),
    'Address' => array(
      'StreetLines' => array(variable_get('commerce_fedex_address_line_1')),
      'City' => variable_get('commerce_fedex_city'),
      'StateOrProvinceCode' => variable_get('commerce_fedex_state'),
      'PostalCode' => variable_get('commerce_fedex_postal_code'),
      'CountryCode' => variable_get('commerce_fedex_country_code'),
    ),
  );
  return $shipper;
}

/**
 * Internal function to build recipient (ship to) array.
 *
 * @param object $order
 *   The order object.
 * @param object $order_wrapper
 *   The order entity wrapper.
 *
 * @return array
 *   An array that represents the ship to address.
 */
function _commerce_fedex_get_recipient($order, $order_wrapper) {
  $field_name = commerce_physical_order_shipping_field_name($order);
  // Prepare the shipping address for use in the request.
  if (!empty($order_wrapper->{$field_name}->commerce_customer_address)) {
    $shipping_address = $order_wrapper->{$field_name}->commerce_customer_address->value();
  }
  else {
    $shipping_address = addressfield_default_values();
  }
  $recipient = array(
    'Contact' => array(
      'PersonName' => $shipping_address['name_line'],
    ),
    'Address' => array(
      'StreetLines' => array($shipping_address['thoroughfare']),
      'City' => $shipping_address['locality'],
      'PostalCode' => $shipping_address['postal_code'],
      'CountryCode' => $shipping_address['country'],
      'Residential' => variable_get('commerce_fedex_shipto_residential', 'residential') == 'residential' ? TRUE : FALSE,
    ),
  );

  // StateOrProvinceCode is only required for shipping to U.S. and Canada.
  if (in_array($shipping_address['country'], array('US', 'CA'))) {
    $recipient['Address']['StateOrProvinceCode'] = $shipping_address['administrative_area'];
  }

  return $recipient;
}

/**
 * Internal function to determine shippable line items from the order.
 *
 * @param object $order
 *   The commerce order object for the order that we're requesting rates for.
 *
 * @return array
 *   The list of packages and dimensions for this order that should be submitted
 *   to FedEx.
 */
function _commerce_fedex_get_package_items($order) {
  // Get the weight and volume of the shippable items.
  $weight = commerce_physical_order_weight($order, 'lb');
  $volume = commerce_physical_order_volume($order, 'in');

  // Determine the default package volume from the FedEx settings.
  $default_package_volume = variable_get('commerce_fedex_default_package_size_length', '0') * variable_get('commerce_fedex_default_package_size_width', '0') * variable_get('commerce_fedex_default_package_size_height', '0');

  // Calculate the number of packages that should be created based on the
  // size of products and the default package volume.
  $number_of_packages = ceil($volume['volume'] / $default_package_volume);

  // Check the FedEx settings to see if we should request insurance.
  $insurance_value = array();
  if (variable_get('commerce_fedex_insurance')) {
    $insurance_value = _commerce_fedex_get_insurance_value($order);
  }

  // Loop through the number of caluclated package to create the return array.
  for ($i = 1; $i <= $number_of_packages; $i++) {
    $package_line_items[$i - 1] = array(
      'SequenceNumber' => $i,
      'GroupPackageCount' => 1,
      'Weight' => array(
        'Value' => round(max(array(0.1, $weight['weight'] / $number_of_packages)), 2),
        'Units' => 'LB',
      ),
      'Dimensions' => array(
        'Length' => variable_get('commerce_fedex_default_package_size_length'),
        'Width' => variable_get('commerce_fedex_default_package_size_width'),
        'Height' => variable_get('commerce_fedex_default_package_size_height'),
        'Units' => 'IN',
      ),
    );

    // If an isurance value has been returned, add insurance value to request.
    if (!empty($insurance_value['amount']) && $insurance_value['amount'] > 0) {
      $package_line_items[$i - 1]['InsuredValue']['Currency'] = $insurance_value['currency_code'];
      $package_line_items[$i - 1]['InsuredValue']['Amount'] = round($insurance_value['amount'] / $number_of_packages, 2);
    }
  }

  // Make sure that we've found shippable items in the order.
  if (!empty($package_line_items)) {
    return $package_line_items;
  }
  else {
    return FALSE;
  }
}

/**
 * Internal function to caluclate insurance value of shippable line items.
 *
 * @param object $order
 *   The commerce order object for the order that we're requesting rates for.
 *
 * @return array
 *   The total value of shippable items in the order for insurance.
 */
function _commerce_fedex_get_insurance_value($order) {
  $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
  $order_total = $order_wrapper->commerce_order_total->value();
  $insurance_value = 0;

  // Loop over each line item on the order.
  foreach ($order_wrapper->commerce_line_items as $line_item_wrapper) {

    // Only collect value of product line items that are shippable.
    if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())
        && commerce_physical_line_item_shippable($line_item_wrapper->value())) {
      $line_item_total = $line_item_wrapper->commerce_total->value();

      // Increment the insurance value from the line items value.
      $insurance_value += commerce_currency_amount_to_decimal($line_item_total['amount'], $line_item_total['currency_code']);
    }
  }

  // Return the insurance value and currency code of shippable items.
  return array(
    'amount' => $insurance_value,
    'currency_code' => $order_total['currency_code'],
  );
}
