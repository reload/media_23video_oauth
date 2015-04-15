<?php

function media_23video_oauth_applications() {
  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');
  $query->orderBy('m.site_id', 'ASC');
  $res = $query->execute();
  $row_count = $res->rowCount();

  while ($record = $res->fetchAssoc()) {
    dpm($record);
  }
}

function media_23video_oauth_authenticate() {
  $form = array();

  $form['consumer']['consumer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Key'),
  );

  $form['consumer']['consumer_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Consumer Secret'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Go',
  );

  return $form;
}

function media_23video_oauth_authenticate_validate($form, &$form_state) {
  if (!isset($form_state['values']['consumer_key']) || empty($form_state['values']['consumer_key'])) {
    form_set_error('consumer_key', t('Consumer key must be set'));
  }
  if (!isset($form_state['values']['consumer_secret']) || empty($form_state['values']['consumer_secret'])) {
    form_set_error('consumer_secret', t('Consumer key must be set'));
  }

  $query = db_select('media_23video_oauth', 'm');
  $query->fields('m');
  $query->condition('m.consumer_key', $form_state['values']['consumer_key']);
  $query->condition('m.consumer_secret', $form_state['values']['consumer_secret']);
  $res = $query->execute();

  if ($res->rowCount() > 0) {
    form_set_error('consumer', t('Consumer key/secret pair already exists'));
  }
}

function media_23video_oauth_authenticate_submit($form, &$form_state) {
  global $base_url;

  $oauth = new OAuth($form_state['values']['consumer_key'], $form_state['values']['consumer_secret']);
  $request = $oauth->getRequestToken('http://api.visualplatform.net/oauth/request_token', $base_url . '/media-23video-oauth-callback');

  if ($request['oauth_callback_confirmed'] == TRUE) {
    media_23video_oauth_db_add($form_state['values']['consumer_key'], $form_state['values']['consumer_secret'], $request['oauth_token'], $request['oauth_token_secret']);
    $form_state['redirect'] = 'http://api.visualplatform.net/oauth/authorize?oauth_token=' . check_plain($request['oauth_token']);
  }
}

function media_23video_oauth_authenticate_callback() {
  global $base_url;
  $params = drupal_get_query_parameters();

  $application = media_23video_oauth_db_get_by_oauth_token($params['oauth_token']);
  if (!$application) {
    return '';
  }

  $oauth = new OAuth($application['consumer_key'], $application['consumer_secret']);
  $oauth->setToken($application['oauth_token'], $application['oauth_token_secret']);

  try {
    $response = $oauth->getAccessToken('http://api.visualplatform.net/oauth/access_token', NULL, $params['oauth_verifier']);
  }
  catch (OAuthException $e) {
    drupal_set_message(t('Exception raised while trying to get access token from 23 Video Site'), 'error');
    watchdog('media_23video_oauth', 'Exception: ' . $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  media_23video_oauth_db_update($application['consumer_key'], $application['consumer_secret'], $response['oauth_token'], $response['oauth_token_secret'], $response['domain'], $response['user_id'], $response['user_name'], $response['user_email'], $response['site_id'], $response['site_name']);
}