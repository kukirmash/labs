<?php
/**
 * @package Joomla.Site
 * @subpackage mod_ut_tiktokembed
 * @author Unitemplates https://www.unitemplates.com
 * @copyright Copyright (C) 2025 - 2025 Unitemplates All rights reserved.
 * @license GNU General Public License version 2 or later; see LICENSE.txt
 */

defined('_JEXEC') or die;

use Joomla\CMS\Language\Text;
?>
<div class="ut-tiktokembed-container <?php echo $moduleclass_sfx; ?>">
    <?php if (!empty($embedHtml) && strpos($embedHtml, 'ERROR:') !== 0) : ?>
        <?php echo $embedHtml; ?>
        <script async src="https://www.tiktok.com/embed.js"></script>
    <?php else : ?>
        <div class="tiktok-embed-error alert alert-warning">
            <?php if (!empty($embedHtml) && strpos($embedHtml, 'ERROR:') === 0) : ?>
                <?php 
                // Show Tiktok error message with custom message
                $tikTokMessage = substr($embedHtml, 6); // remove 'ERROR: ' from start
                echo $tikTokMessage. '. <strong>' .Text::_('MOD_UT_TIKTOKEMBED_ERROR_MESSAGE').'</strong>'; 
                ?>
            <?php else : ?>
                <?php echo Text::_('MOD_UT_TIKTOKEMBED_ERROR_UKNOWN_MESSAGE'); ?>
            <?php endif; ?>
        </div>
    <?php endif; ?>
</div>