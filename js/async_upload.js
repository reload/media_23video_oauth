/**
 * @file
 * Javascript for proper handling of uploading files to a 23Video server.
 */
(function ($) {
  /**
   * Get upload token synchronously from Drupal site.
   */
  function get_upload_token() {
    $('#upload-status').text(Drupal.t('Fetching upload token...'));
    var title = $('#media-23video-oauth-upload input[name="title"]').val();
    var description = $('#media-23video-oauth-upload input[name="description"]').val();
    var album_id = $('#media-23video-oauth-upload input[name="album_id"]').val();
    $('#media-23video-oauth-upload').find('input[name="title"], input[name="description"]').attr('disabled', 'disabled');
    $.ajax({
      url: '/media-23video-uploadtoken/',
      data: {'title': encodeURI(title), 'description': encodeURI(description), 'album_id': encodeURI(album_id)},
      dataType: 'json',
      async: true,
      success: function (responseText, statusText, xhr) {
        if (responseText.upload_token) {
          $('#upload-status').text('Uploading file.');
          // Save upload token in input tag.
          $('input[name="upload_token"]').val(responseText.upload_token);
          $('#media-23video-oauth-upload').submit();
          return true;
        }
        else {
          $('#upload-status').text('An error ocurred while fetching upload token. Try again later.');
          $('#media-23video-oauth-upload .ajax-progress-throbber div').removeAttr('class');
          $('#media-23video-oauth-upload').find('input[name="title"], input[name="description"]').removeAttr('disabled');
          alert(Drupal.t("Error. Couldn't receive upload token. "));
          return false;
        }
      },
      error: function (xhr, statusText, errorThrown) {
        $('#upload-status').text('An error ocurred. Try again later.');
        $('#media-23video-oauth-upload .ajax-progress-throbber div').removeAttr('class');
        $('#media-23video-oauth-upload').find('input[name="title"], input[name="description"]').removeAttr('disabled');
        switch (statusText) {
          case 'timeout':
            alert(Drupal.t('The server timed out. Try again.'));
            break;
          case 'error':
            alert(Drupal.t('An error occurred.'));
            break;
          case 'abort':
            alert(Drupal.t('Upload was aborted.'));
            break;
          default:
            alert(Drupal.t('Unknown error occurred.'));
            break;
        }
        return false;
      }
    })
  }

  /**
   * Upload files via ajax.
   *
   * @type {{attach: Function}}
   */
  Drupal.behaviors.upload = {
    attach: function (context, settings) {
      $('#media-23video-oauth-upload').find('#edit-submit').click(function() {
        $('.ajax-progress-throbber div').attr('class', 'throbber');
        get_upload_token();
      });
      $("#edit-description, #edit-title").keypress(function(event) {
        if (event.keyCode == 13) {
          $('.ajax-progress-throbber div').attr('class', 'throbber');
          get_upload_token();
          return false;
        }
      });
      var options = {
        dataType: 'json',
        async: true,
        beforeSubmit: function(arr, $form, options) {
          if ($('#edit-upload-file').val() == '') {
            $('#upload-status').text('File is empty.');
            $('#media-23video-oauth-upload .ajax-progress-throbber div').removeAttr('class');
            $('#media-23video-oauth-upload').find('input[name="title"], input[name="description"]').removeAttr('disabled');
            return false;
          }
        },
        success: function(responseText, statusText, xhr, $form) {
          $("input[name='photo_id']").val(responseText.photo_id);
          $("input[name='title']").val(responseText.title);
          $("input[name='token']").val(responseText.token);
          $("#media-23video-oauth-attach").submit();
          return true;
        },
        error: function (xhr, statusText, errorThrown) {
          $('#upload-status').text('An error ocurred. Try again later.');
          $('#media-23video-oauth-upload .ajax-progress-throbber div').removeAttr('class');
          $('#media-23video-oauth-upload').find('input[name="title"], input[name="description"]').removeAttr('disabled');
          switch (statusText) {
            case 'timeout':
              alert(Drupal.t('The server timed out. Try again.'));
              break;
            case 'error':
              alert(Drupal.t('An error occurred.'));
              break;
            case 'abort':
              alert(Drupal.t('Upload was aborted.'));
              break;
            default:
              alert(Drupal.t('Unknown error occurred.'));
              break;
          }

          return false;
        }
      };

      // Use upload form as an ajax form.
      $('#media-23video-oauth-upload').ajaxForm(options);
    }
  };

})(jQuery);
