/**
 * @file
 * Make the drag and drop element toggle it's message on mouseover.
 */
(function($) {
  Drupal.behaviors.rawphenoToggleDragnDropMessage = {
    attach: function (contex, settings) {
      $(document).ready(function() { 

		    $('.droppable-browse-button').text('choose your file');
		    // Drag over events.
		    var drop = document.getElementById('droppable-dnd');
        var dropMessageHTML = $('.droppable-message').html();
        var dropMessage = $('.droppable-message');
        
        if (drop) {
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
        }        
        
      });
    }
  };
}(jQuery));

/**
 * Diable the next step button until a success message with an id=rawpheno-upload-successful
 * appears on the page.
 */
(function($) {
  Drupal.behaviors.rawphenoUpload1ControlWorkflow = {
    attach: function (contex, settings) {
      $(document).ready(function() { 

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
      });
    }
  };
}(jQuery));
