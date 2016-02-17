/**
 * @file
 * Managed form behaviors in stage 02 - review new traits.
 */
(function($) {
  Drupal.behaviors.rawphenoUploadStage02 = {
    attach: function (contex, settings) {
      // Reference checkbox element.
      var chkNewTrait = $('input.form-checkbox');
      // Show form for collecting more information.
      chkNewTrait.each(function () {
        if( this.checked ) {
          var x = this.getAttribute('id');
          x = '#div-'+x;
          $(x).show();
        }
      });
      
      // On error, if checkbox is checked, show form.
      chkNewTrait.click(function () {
        var x = this.getAttribute('id');
        x = '#div-'+x;
        if( this.checked )
          $(x).show();
        else
          $(x).hide();
      });
      
      // Show error near the stage indicator.
      if( document.getElementById('messages') ) {
        // Check for durpal error message.
        $('#messages').remove();
        $('#stage02-window-error').show();
      }
    }
  };
}(jQuery));
