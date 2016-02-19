/**
 * @file
 * Manage navigation buttons in stage 01 - upload.
 */
(function($) {
  Drupal.behaviors.rawphenoUploadStage01 = {
    attach: function (contex, settings) {
      $(document).ready(function() { 
        // Reference form buttons.
        var btnClose = $('#edit-stage01-button-close');
        var btnSave = $('#edit-stage01-submit-save');
        var btnReview = $('#edit-stage01-submit-review');
      
        // Hide buttons in stage 01.
        btnClose.hide();
        btnSave.hide();
        btnReview.hide();
 
        // When file upload encounters an error - include a close button
        // to allow user to close error window and try again.
        btnClose.click(function () { 
          btnClose.hide();
          $('div.messages').remove();
          $('#edit-dnd-ajax-wrapper').slideDown('fast');
        });
      
        // Determine which button to display based on the result
        // of the upload. A Save spreadsheet button when upload is successful
        // and a describe trait button when new traits need review.
        if (document.getElementById('stage01-window-success')) {
          // File is valid.
          btnSave.show();
        }
        else if (document.getElementById('stage01-window-review')) {
          // File has new headers.
          btnReview.show();
        }
      
        // Disable dropzone when there is error.
        if (document.getElementById('stage01-window-error')) {
			     $('#edit-dnd-ajax-wrapper').hide(); 
		       //Show close window button.
		       btnClose.show();
		    }
		  
		    $('.droppable-browse-button').text('choose your file');
		    // Drag over events.
		    var drop = document.getElementById('droppable-dnd');
        var dropMessageHTML = $('.droppable-message').html();
        var dropMessage = $('.droppable-message');
        
        drop.addEventListener("dragover", function() { 
          drop.style.border = '3px dashed #999999';
          drop.style.backgroundColor = '#FBFBFB';
          // Create a new instruction to user.
          dropMessage.text('Drop to upload and validate your spreadsheet'); 
        });  
      
        drop.addEventListener("dragleave", function() { 
          drop.style.border = '3px solid #EAEAEA';
          drop.style.backgroundColor = '#FAFAFA';
          // Remove new instruction and restore original message.
          dropMessage.html(dropMessageHTML);
        });
        
        drop.addEventListener("drop", function() { 
          drop.style.border = '3px solid #EAEAEA';
          drop.style.backgroundColor = '#FAFAFA';
          // Remove new instruction and restore original message.
          dropMessage.html(dropMessageHTML);
        });
        
        
      });
    }
  };
}(jQuery));
