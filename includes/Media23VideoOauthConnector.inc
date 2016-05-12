<?php

/**
 * @file
 * Class for using 23Video's API.
 *
 * @see http://www.23video.com/api/
 */

/**
 * Class Media23VideoOauthConnector.
 *
 * Wrapper for communicating with 23Video's server via OAuth.
 */
class Media23VideoOauthConnector {
  protected static $instance;
  protected $consumer;
  public $error;

  /**
   * Singleton.
   */
  public static function getInstance() {
    // Load oauth library.
    $library = libraries_load('23video-for-php');
    if (isset($library['error'])) {
      $class = __CLASS__;
      self::$instance = new $class();
      self::$instance->error = $library['error message'];
      return self::$instance;
    }

    $oauth = media_23video_oauth_get_user_id();

    $consumer_key_public = $oauth['consumer_key'];
    $consumer_key_secret = $oauth['consumer_secret'];
    $access_token_public = $oauth['oauth_token'];
    $access_token_secret = $oauth['oauth_token_secret'];
    $site_url = 'http://' . $oauth['domain'];

    // Set up OAuth request.
    $consumer = new VisualVideo($site_url, $consumer_key_public, $consumer_key_secret, $access_token_public, $access_token_secret);

    if (!isset(self::$instance)) {
      $class = __CLASS__;
      self::$instance = new $class();
      self::$instance->setConsumer($consumer);
    }
    return self::$instance;
  }

  /**
   * Perform GET request with json format with raw = 1.
   *
   * @param string $api_function
   *   Api function, e.g., "/api/photo/list".
   * @param array $params
   *   Parameters to web service method.
   *
   * @return array
   *   Return json-decoded array
   */
  protected function doRequest($api_function, $params = array()) {
//    file_put_contents('/tmp/test.txt', print_r(array(
//      'api_function' => $api_function, 'params' => $params
//    ), 1));
    $consumer = $this->getConsumer();
    $params['format'] = 'json';
    $params['raw'] = 1;

    $response = $consumer->get($api_function, $params);
    return $this->getResponseJson($response);
  }

  /**
   * Set consumer (VisualVideo instance).
   *
   * @param VisualVideo $consumer
   *   VisualVideo instance.
   */
  public function setConsumer(VisualVideo $consumer) {
    if ($consumer && $consumer instanceof VisualVideo) {
      $this->consumer = $consumer;
    }
  }

  /**
   * Get consumer.
   *
   * @return VisualVideo
   *   VisualVideo instance
   */
  public function getConsumer() {
    return $this->consumer;
  }

  /**
   * Get all photos.
   *
   * @param bool $flush_cache
   *   Do we want to flush the cache.
   *
   * @return array
   *   json-decoded array
   */
  public function doPhotoListAll($flush_cache = FALSE) {
    return $this->doPhotoList(0, $flush_cache);
  }

  /**
   * Get list of photos.
   *
   * @param int $photo_id
   *   Photo id.
   * @param bool $flush_cache
   *   If we want to flush the cache.
   *
   * @return array
   *   Json-decoded array
   */
  public function doPhotoList($photo_id = NULL, $flush_cache = FALSE) {
    $cache = cache_get(__CLASS__ . ':' . __FUNCTION__ . ':' . $photo_id);

    if ($flush_cache == TRUE || empty($cache)) {
      $api_function = '/api/photo/list';

      $params = array();
      if ($photo_id) {
        $params['photo_id'] = $photo_id;
      }

      $response = $this->doRequest($api_function, $params);

      if ($response['status'] == 'ok') {
        cache_set(__CLASS__ . ':' . __FUNCTION__ . ':' . $photo_id, $response);
      }
    }
    else {

      $info = $cache->data;

    }
    return $info;
  }

  /**
   * Get list of albums.
   *
   * @return array
   *   JSon-decoded array
   */
  public function doAlbumList($flush_cache = FALSE) {
    $cache = cache_get(__CLASS__ . ':' . __FUNCTION__);

    if ($flush_cache || empty($cache)) {
      $api_function = '/api/album/list';
      $response = $this->doRequest($api_function, array());

      if ($response['status'] == 'ok') {
        cache_set(__CLASS__ . ':' . __FUNCTION__, $response);
      }
    }
    else {
      $response = $cache->data;
    }

    return $response;
  }

  /**
   * Get section list.
   *
   * @param int $photo_id
   *   Photo (video) id.
   * @param string $token
   *   Token for photo (video).
   *
   * @return array
   *   JSON-decoded array
   */
  public function doSectionList($photo_id, $token) {
    $params = array(
      'photo_id' => $photo_id,
      'token' => $token,
    );
    $api_function = '/api/photo/section/list';
    return $this->doRequest($api_function, $params);
  }

  /**
   * Create section of a video.
   *
   * @param int $photo_id
   *   Photo (video) id.
   * @param int $start_time
   *   Seconds from start.
   * @param string $title
   *   Title of section.
   * @param string $description
   *   Description of section.
   *
   * @return array
   *   JSON-decoded array
   */
  public function doSectionCreate($photo_id, $start_time, $title = 'Start of video', $description = 'Start of video') {
    $params = array(
      'photo_id' => $photo_id,
      'start_time' => $start_time,
      'title' => $title,
      'description' => $description,
    );
    $api_function = '/api/photo/section/create';
    return $this->doRequest($api_function, $params);
  }

  /**
   * Get upload token.
   *
   * @param string $title
   *   Title of video.
   * @param string $description
   *   Description of video.
   * @param int $album_id
   *   Album id to attach video to.
   * @param string $return_url
   *   Callback url after upload of file.
   *
   * @return array
   *   JSON-decoded array
   */
  public function doGetUploadToken($title, $description, $album_id, $return_url) {
    $api_function = '/api/photo/get-upload-token';
    $params = array(
      'title' => $title,
      'description' => $description,
      'album_id' => $album_id,
      'return_url' => $return_url,
    );
    $response = $this->doRequest($api_function, $params);
    return $response['uploadtoken']['upload_token'];
  }

  /**
   * Get user login token.
   *
   * @param int $user_id
   *   User id.
   * @param string $return_url
   *   URL to be redirected to after login.
   *
   * @return string
   *   JSON formatted.
   */
  public function doGetLoginToken($user_id, $return_url) {
    $api_function = '/api/user/get-login-token';
    $params = array(
      'user_id' => $user_id,
      'return_url' => $return_url,
    );
    $response = $this->doRequest($api_function, $params);

    return $response;
  }

  /**
   * Get session token.
   *
   * @param string $return_url
   *   URL to be returned to after login.
   * @param string $email
   *   Email address of logged in user.
   * @param string $full_name
   *   Full name of logged in user.
   *
   * @return array
   *   JSON-decoded array
   */
  public function doGetSessionToken($return_url, $email = '', $full_name = '') {
    $api_function = '/api/session/get-token';
    $params = array(
      'return_url' => $return_url,
    );
    if (!empty($email)) {
      $params['email'] = $email;
    }
    if (!empty($full_name)) {
      $params['full_name'] = $full_name;
    }

    $response = $this->doRequest($api_function, $params);
    return $response;
  }

  /**
   * Get user list.
   *
   * @return string
   *   JSON formatted list of users.
   */
  public function doUserList() {
    $api_function = '/api/user/list';
    return $this->doRequest($api_function, array());
  }

  /**
   * Decode JSON response to array.
   *
   * @param string $body
   *   JSON from 23Video server.
   *
   * @return array
   *   JSON-decoded array
   */
  public function getResponseJson($body) {
    $info = drupal_json_decode($body);
    if ($info['status'] != 'ok') {
      // TODO: error handling.
    }
    return $info;
  }

}
