<?php
/**
 * @package Joomla.Site
 * @subpackage mod_ut_tiktokembed
 * @author Unitemplates https://www.unitemplates.com
 * @copyright Copyright (C) 2025 - 2025 Unitemplates All rights reserved.
 * @license GNU General Public License version 2 or later; see LICENSE.txt
 */

namespace Joomla\Module\UtTiktokEmbed\Site\Helper;

defined('_JEXEC') or die;

use Joomla\CMS\Http\HttpFactory;

/**
 * Helper for mod_ut_tiktokembed
 * @since  1.0.0
 */
class TiktokEmbedHelper
{
    /**
     * Get the embed HTML for a TikTok URL using the oEmbed API
     * @param   string  $url     The TikTok URL
     * @param   string  $type    The embed type (single or list)
     * @param   string  $list  The list URLs (for list type)
     * @return  string|false  The embed HTML or false on failure
     * @since   1.0.0
     */
    public static function getEmbed($url, $type = 'single', $list = '')
    {
        // Check embed type and process accordingly
        if ($type === 'list' && !empty($list)) {
            return self::processList($list);
        }
        
        // For single embed type
        // Validate URL
        if (!$url || strpos($url, 'tiktok.com') === false) {
            return false;
        }

        // Ensure URL uses HTTPS
        $url = str_replace('http://', 'https://', $url);

        // Build resquest URL
        $apiUrl = 'https://www.tiktok.com/oembed?url=' . urlencode($url);
                
        try {
            // Get Response
            $http = HttpFactory::getHttp();
            $response = $http->get($apiUrl);
            if ($response->code !== 200) {
                // Getting error message from TikTok
                $errorData = json_decode($response->body);
                if (isset($errorData->message)) {
                    return 'ERROR: ' . $errorData->message;
                }
                // Or show a generic error message
                return 'ERROR: Uknown TikTok error';
            }
            $data = json_decode($response->body);
            
            // Get the HTML and ensure it uses HTTPS
            if (isset($data->html)) {
                $html = $data->html;                
                $html = str_replace('http://', 'https://', $html);                
                return $html;
            }
            
            return false;
        } catch (\Exception $e) {
            return false;
        }
    }
    
    /**
     * Process a list of TikTok URLs and generate embed code
     * @param   string  $list  The list URLs (one per line)
     * @return  string  The embed HTML for the list
     * @since   1.0.0
     */
    public static function processList($list)
    {
        // Split the list into individual URLs
        $urls = preg_split('/\r\n|\r|\n/', $list);
        $videoIds = [];
        
        // Extract video IDs from each URL
        foreach ($urls as $url) {
            $url = trim($url);
            if (empty($url)) {
                continue;
            }
            
            // Extract video ID using regex
            if (preg_match('/\/video\/([0-9]+)/', $url, $matches)) {
                $videoIds[] = $matches[1];
            }
        }
        
        // If no valid video IDs found
        if (empty($videoIds)) {
            return 'ERROR: No valid TikTok video URLs found';
        }
        
        // Generate the list embed code
        $videoIdList = implode(',', $videoIds);
        $html = '<blockquote class="tiktok-embed" '
             . 'cite="https://www.tiktok.com" '
             . 'data-embed-type="curated" '
             . 'data-video-id-list="' . $videoIdList . '" '
             . 'data-embed-from="embed_page" '
             . 'style="max-width: 780px;min-width: 325px;">' 
             . '<section>' 
             . '<a target="_blank" href="https://www.tiktok.com?refer=embed_page">TikTok</a>' 
             . '</section>' 
             . '</blockquote>';
        
        return $html;
    }
}