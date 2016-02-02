/**
 * @file
 * Manage navigation buttons in stage 01 - upload.
 *
 */
(function($) {
  Drupal.behaviors.rawphenoUploadStage01 = {
    attach: function (contex, settings) {
      //Reference to buttons.
      var btnClose = $('#edit-stage01-button-close');
      var btnSave = $('#edit-stage01-submit-save');
      var btnReview = $('#edit-stage01-submit-review');
      
      //Hide buttons in stage 01.
      btnClose.hide();
      btnSave.hide();
      btnReview.hide();
 
      //When upload encounters an error - include a close button
      //to allow user to close reported errors and try again.
      btnClose.click(function () { 
        btnClose.hide();
        $('div.messages').remove();
        $('#edit-dnd-ajax-wrapper').slideDown('fast');
      });
      
      //Determine which button to display based on the result
      //of the upload. Save spreadsheet when upload is successful
      //and describe trait when new traits need review.
      if( document.getElementById('stage01-window-success') ) {
        //xls is valid, show save spreadsheet button
        btnSave.show();
      }
      else if( document.getElementById('stage01-window-review') ) {
        //else, show describe and save button
        btnReview.show();
      }
      
      //Disable dropzone when there is error.
      if( document.getElementById('stage01-window-error') ) {
			   $('#edit-dnd-ajax-wrapper').hide(); 
		     //Show close window button
		     btnClose.show();
		  }
    }
  };
}(jQuery));