(function($) {
  Drupal.behaviors.rawphenoUploadStage01 = {
    attach: function (contex, settings) {
      //ref buttons
      var btnClose = $('#edit-stage01-button-close');
      var btnSave = $('#edit-stage01-submit-save');
      var btnReview = $('#edit-stage01-submit-review');
 
      btnClose.hide();
      btnSave.hide();
      btnReview.hide();
 
      //close error window
      btnClose.click(function () { 
        btnClose.hide();
        $('div.messages').remove();
        $('#edit-dnd-ajax-wrapper').slideDown('fast');
      });
      
      if( document.getElementById('stage01-window-success') ) {
        //xls is valid, show save spreadsheet button
        btnSave.show();
      }
      else if( document.getElementById('stage01-window-review') ) {
        //else, show describe and save button
        btnReview.show();
      }
      
      //hide dropzone when there is an error
      if( document.getElementById('stage01-window-error') ) {
			   $('#edit-dnd-ajax-wrapper').hide(); 
		     //show close window button
		     btnClose.show();
		  }
    }
  };
}(jQuery));