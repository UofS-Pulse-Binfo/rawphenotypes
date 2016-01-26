(function($) {
  Drupal.behaviors.rawphenoUploadStage02 = {
    attach: function (contex, settings) {
      var chkNewTrait = $('input.form-checkbox');
      chkNewTrait.each(function () {
        if( this.checked ) {
          var x = this.getAttribute('id');
          x = '#div-'+x;
          $(x).show();
        }
      });
      
      //on error, if checkbox is checked, default to open
      chkNewTrait.click(function () {
        var x = this.getAttribute('id');
        x = '#div-'+x;
        if( this.checked )
          $(x).show();
        else
          $(x).hide();
      });
      
      if( document.getElementById('messages') ) {
        //check for durpal error message
        $('#messages').remove();
        $('#stage02-window-error').show();
      }
    }
  };
}(jQuery));