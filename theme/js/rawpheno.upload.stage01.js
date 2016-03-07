/**
 * @file
 * Make the drag and drop element toggle it's message on mouseover.
 */
(function($) {
  Drupal.behaviors.rawphenoToggleDragnDropMessage = {
    attach: function (contex, settings) {
//<<<<<<< Updated upstream=======<<<<<<< HEAD      $(document).ready(function() { 
// 		    $('.droppable-browse-button').text('choose your file');
//=======>>>>>>> Stashed changes
        // Hide the next button unless needed.
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
        
        // Handle the hover-over effects of the drop-zone.
		    $('.droppable-browse-button').text('choose your file');

		    // Drag and drop dropzone container div.
		    var drop = document.getElementById('droppable-dnd');
        // Inner container div that holds controls, labels and links.
        var dropMessage = $('.droppable-message');
        var dropMessageHTML = $('.droppable-message').html();
        
        // Add corresponding message on each mouse/drag event.
        if (drop) {
          drop.addEventListener("dragover", function() { 
            dropMessage.css('border','3px dashed #AAAAAA');
            // Create a new instruction to user.
            dropMessage.text('Drop to upload and validate your spreadsheet'); 
          });  
      
          drop.addEventListener("dragleave", function() { 
            dropMessage.css('border','none');
            // Remove new instruction and restore original message.
            dropMessage.html(dropMessageHTML);
          });
        
          drop.addEventListener("drop", function() { 
            dropMessage.css('border','none');
            // Clear dropzone from text information as AJAX upload starts.
            dropMessage.html('');
          });
        }  

        // Allow user to collapse upload instructions message box.
        $('hr').click(function() {
          if ($(this).attr('class') == 'icon-up') {
            // Help window text open.
            $('#txt-help-text').animate({height : '140px'}, 300, function() { 
              $('hr').removeClass('icon-up');
            });
          } 
          else {
            // Help window text collapse.
            $('#txt-help-text').animate({height : '0px'}, 300, function() { 
              $('hr').addClass('icon-up');
            });
          }
        });
      });
    }
  };
}(jQuery));
