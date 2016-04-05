/**
 * Make the drag and drop element toggle it's message on mouseover.
 */
(function($) {
  Drupal.behaviors.ToggleDragnDropMessage = {
    attach: function (context, settings) {
      // Replace button value from browse to choose your file.
	    $('.droppable-browse-button').text('choose your file');
	  
	    // Drop area element.
	    var dropZone = document.getElementById('droppable-dnd');
	    // Initial/default meassage of the drop area.
	    var dropMessageHTML = $('.droppable-message').html();
	    var dropMessage = $('.droppable-message');
	   
	    // Add corresponding message on mouse event.
	    if (dropZone) {
	      // User drags file into the drop area.
		    dropZone.addEventListener('dragover', function() {
		      dropMessage.css('border','3px dashed #AAAAAA');
          // Create a new instruction to user.
          dropMessage.html('<span>Drop to upload and validate your spreadsheet</span>'); 										
	  	  });
	    
		    // User cancels file drop.
		    dropZone.addEventListener('dragleave', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          dropMessage.html(dropMessageHTML); 										
		    });
		
		    // User drops file.
		    dropZone.addEventListener('drop', function() {
		      dropMessage.css('border','none');
          // Create a new instruction to user.
          dropMessage.html(dropMessageHTML); 		
          
          if ($('div.file-upload-js-error').length > 0) {
            // AJAX dies or is frozen after an error.
            if ($('div.messages').length > 1) {
              // Remove validation result and drupal error message.
              $('div.messages')[0].remove();
            }
          }
          else {
            // Remove any messages.
            $('div.messages').remove();
          }
		    });
	    
	      // For security, suppress any alert message showing snippet of code or 
	      // module settings.     
	      alert = function() {
          // Clear dropzone stage and set it to default message.
          $('div.droppable-preview-file').hide();
          dropMessage.html(dropMessageHTML);
          // Create an error message.
          $('<div class="messages error">The specified file is not a valid Microsoft Excel File.</div>').insertAfter('hr');
        };
	    } 
    }
  };
}(jQuery));

/**
 * Diable the next step button until a success message with an id=rawpheno-upload-successful
 * appears on the page.
 */
(function($) {
  Drupal.behaviors.rawphenoUpload1ControlWorkflow = {
    attach: function (context, settings) {
      var submitButton = $('#edit-next-step');
      var successMsg = $('#rawpheno-upload-successful');
        
      if (successMsg.length) {
        submitButton.removeClass('form-button-disabled');
        submitButton.removeAttr('disabled');
      }
      else {
        submitButton.addClass('form-button-disabled');
        submitButton.attr('disabled','disabled');
      }
    }
  };
}(jQuery));