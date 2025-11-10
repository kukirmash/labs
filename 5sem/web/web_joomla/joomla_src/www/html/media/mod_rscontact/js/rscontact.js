var RSContact = {
	init: function (uniqid) {
		var $container = jQuery("#mod-rscontact-container-" + uniqid);

		$container.find("#mod-rscontact-message-" + uniqid).each(function(){
			var msg_len = jQuery(this).attr('maxlength');

			jQuery('<p><small id="mod-rscontact-counter-' + uniqid + '">' + Joomla.JText._('MOD_RSCONTACT_CHARACTERS_LEFT').replace('%d', msg_len) + '</small></p>').insertAfter(this);

			jQuery(this).keyup(function(){
				jQuery("#mod-rscontact-counter-" + uniqid).html(Joomla.JText._('MOD_RSCONTACT_CHARACTERS_LEFT').replace('%d', RSContact.textCounter(this, msg_len)));
			});
		});

		if (typeof jQuery.fn.validate === 'function')
		{
			$container.find("#mod-rscontact-contact-form-" + uniqid).validate();
		}
		else
		{
			console.log('jQuery.fn.validate not found.');
		}
	},

	textCounter: function (field, maxlimit) {
		return maxlimit - field.value.length;
	}
}

jQuery(document).ready(function()
{
	if (typeof jQuery.validator === 'undefined')
	{
		console.log('jQuery.validator not found. Make sure jquery.validate.js is loaded.');
		return;
	}
	jQuery.validator.setDefaults({
		errorClass: "error",
		labelErrorClass: "mod-rscontact-error",
		validClass: "success",
		ignore: ".ignore",
		focusInvalid: true,
		highlight: function(element, errorClass) {
			jQuery(element).fadeIn();
		},
		rules: {
			mod_rscontact_salutation: "required",
			mod_rscontact_first_name: {
				required: true,
				minlength: 2
			},
			mod_rscontact_last_name: {
				required: true,
				minlength: 2
			},
			mod_rscontact_full_name: {
				required: true,
				minlength: 2
			},
			mod_rscontact_email: "required",
			mod_rscontact_address_1: {
				required: true,
				minlength: 2
			},
			mod_rscontact_address_2: {
				required: true,
				minlength: 2
			},
			mod_rscontact_city: {
				required: true,
				minlength: 2
			},
			mod_rscontact_states: "required",
			mod_rscontact_zip: {
				required: true,
				alphanumeric: true,
				minlength: 2
			},
			mod_rscontact_home_phone: {
				required: true,
				digits: true,
				minlength: 4
			},
			mod_rscontact_mobile_phone: {
				required: true,
				digits: true,
				minlength: 4
			},
			mod_rscontact_work_phone: {
				required: true,
				digits: true,
				minlength: 4
			},
			mod_rscontact_company: "required",
			mod_rscontact_website: {
				required: true,
				url: true
			},
			mod_rscontact_subject: {
				required: true
			},
			mod_rscontact_message:  {
				required: true,
				minlength: 2
			},
			mod_rscontact_cf1:  {
				required: true
			},
			mod_rscontact_cf2:  {
				required: true
			},
			mod_rscontact_cf3:  {
				required: true
			},
			recaptcha_response_field: "required"
		},
		messages: {
			mod_rscontact_salutation: Joomla.JText._('MOD_RSCONTACT_SALUTATION_ERROR'),
			mod_rscontact_first_name: {
				required: Joomla.JText._('MOD_RSCONTACT_FIRST_NAME_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_last_name: {
				required:  Joomla.JText._('MOD_RSCONTACT_LAST_NAME_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_full_name: {
				required: Joomla.JText._('MOD_RSCONTACT_FULL_NAME_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_email: 		Joomla.JText._('MOD_RSCONTACT_EMAIL_ERROR'),
			mod_rscontact_address_1: {
				required: Joomla.JText._('MOD_RSCONTACT_ADDRESS_1_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_address_2: {
				required: Joomla.JText._('MOD_RSCONTACT_ADDRESS_2_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_city: {
				required: Joomla.JText._('MOD_RSCONTACT_CITY_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_states: 		Joomla.JText._('MOD_RSCONTACT_STATE_ERROR'),
			mod_rscontact_zip: {
				required: Joomla.JText._('MOD_RSCONTACT_ZIP_ERROR'),
				alphanumeric: Joomla.JText._('MOD_RSCONTACT_ZIP_NOT_A_ALPHANUMERIC_ERROR')
			},
			mod_rscontact_home_phone: {
				required: Joomla.JText._('MOD_RSCONTACT_HOME_PHONE_ERROR'),
				digits: Joomla.JText._('MOD_RSCONTACT_PHONE_NOT_A_NUMBER_ERROR')
			},
			mod_rscontact_mobile_phone: {
				required: Joomla.JText._('MOD_RSCONTACT_MOBILE_PHONE_ERROR'),
				digits: Joomla.JText._('MOD_RSCONTACT_PHONE_NOT_A_NUMBER_ERROR')
			},
			mod_rscontact_work_phone: {
				required: Joomla.JText._('MOD_RSCONTACT_WORK_PHONE_ERROR'),
				digits: Joomla.JText._('MOD_RSCONTACT_PHONE_NOT_A_NUMBER_ERROR')
			},
			mod_rscontact_company: {
				required: Joomla.JText._('MOD_RSCONTACT_COMPANY_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_website: {
				required: Joomla.JText._('MOD_RSCONTACT_WEBSITE_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			mod_rscontact_subject: {
				required: Joomla.JText._('MOD_RSCONTACT_SUBJECT_ERROR')
			},
			mod_rscontact_message: {
				required: Joomla.JText._('MOD_RSCONTACT_MESSAGE_ERROR'),
				minlength: Joomla.JText._('MOD_RSCONTACT_MIN_LENGTH_ERROR')
			},
			recaptcha_response_field: ''
		},
		submitHandler: function(validator, event) {
			event.preventDefault();
			var request = [
				'option=com_ajax',
				'module=rscontact',
				'format=jsonpc'
			];
			var id = jQuery(validator).find('[name=mod_rscontact_module_id]').val();
			var values 	= jQuery(validator).serializeArray();

			var name;
			for (var i = 0; i < values.length; i++)
			{
				if (values[i].name.indexOf('mod_rscontact_') === 0)
				{
					if (values[i].name.indexOf('[]') > -1)
					{
						name = 'data[' + encodeURIComponent(values[i].name.substring(0, values[i].name.length - 2)) + '][]';
					}
					else
					{
						name = 'data[' + encodeURIComponent(values[i].name) + ']';
					}

					request.push(name + '=' + encodeURIComponent(values[i].value))
				}
				else
				{
					request.push(encodeURIComponent(values[i].name)  + '=' + encodeURIComponent(values[i].value))
				}
			}

			jQuery(validator).find('#mod-rscontact-submit-btn-' + id).attr("disabled", "disabled");
			jQuery(validator).find('.icon').removeAttr('class').addClass('icon icon-refresh rstpl-rotating');

			jQuery.ajax({
				type   : 'POST',
				data   : request.join('&'),
				success: function (response){
					response = JSON.parse(response);
					if(response.status == 0) {
						jQuery('#mod-rscontact-error-msg-' + id).hide().html('<div class="alert alert-error alert-danger">' + response.message + '</div>').fadeIn().delay(1000).fadeOut(5000);
						jQuery(validator).find('#mod-rscontact-submit-btn-' + id).removeAttr("disabled");
						jQuery(validator).find('.icon').removeAttr('class').addClass('icon icon-envelope');
					}

					if(response.status == 1){
						jQuery(validator).nextAll('#mod-rscontact-msg-' + id).hide().html('<div class="alert alert-success">' + response.message + '</div>').delay(500).fadeIn();
						if(response.warnings.length > 0){
							jQuery(validator).nextAll('#mod-rscontact-warning-msg-' + id).hide().html('<div class="alert alert-warning"></div>').delay(500).fadeIn();
							jQuery(validator).find('.icon').removeAttr('class').addClass('icon icon-envelope');
							jQuery.each(response.warnings, function(i, value) {
								jQuery(".alert-warning").append(value + '<br />');
							});
						}

						jQuery(validator).fadeOut(500, function(){
							jQuery(this).remove();
						}) ;
						jQuery('html, body').animate({
							scrollTop: jQuery(validator).offset().top-10
						}, 2000);
					}
				},
				error: function (response) {
					response = JSON.parse(response);
					jQuery(validator).find('#mod-rscontact-error-msg-' + id).hide().html('<div class="alert alert-error alert-danger">' + response.message + '</div>').fadeIn().delay(2000).fadeOut(5000);
					jQuery(validator).find('.icon').removeAttr('class').addClass('icon icon-envelope');
				}
			});
			return false;
		}
	});
});
