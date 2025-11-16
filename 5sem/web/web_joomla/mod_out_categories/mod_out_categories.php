<?php

/**
 * @package Joomla.Site
 * @subpackage mod_out_articles
 *
 * @copyright Copyright (C) 2005 - 2016 Open Source Matters,
 * Inc. All rights reserved.
 * @license GNU General Public License version 2 or later;
 * see LICENSE.txt
 */

defined('_JEXEC') or die;

use Joomla\CMS\Factory;

// Получаем значения параметров модуля
$hierarchy_mode = (bool)$params->get('hierarchy_mode', 0);
$title_case_mode = (int)$params->get('title_case_mode', 2);

// Получаем объект базы данных
$db = Factory::getDbo();
$query = $db->getQuery(true);

//Формируем запрос
$query->select([
    $db->quoteName('c.title'),
    $db->quoteName('u.name'),
    $db->quoteName('c.level')
])
    ->from($db->quoteName('#__categories', 'c'))
    ->join('INNER', $db->quoteName('#__users', 'u') .
        'ON' . $db->quoteName('u.id') . '=' . $db->quoteName('c.created_user_id'))
    ->order($db->quoteName('u.name'));

// Предварительно устанавливаем текст запроса
$db->setQuery($query);

// Выолняем запрос
$db->execute();

// Получаем результат - индексированный массив, 
// каждый элемент которого является объект.
$data = $db->loadObjectList();

// Функция форматирования заголовка
function formatTitle($title, $mode)
{
    switch ($mode) {
        case 0: // Все прописные
            return mb_strtoupper($title);
        case 1: // Все строчные
            return mb_strtolower($title);
        case 2: // Первая буква прописная, остальные строчные
            $title = mb_strtolower($title);
            $first = mb_strtoupper(mb_substr($title, 0, 1));
            return $first . mb_substr($title, 1);
        default:
            return $title;
    }
}

// Вывод таблицы
if (!empty($data)) {
    echo '<table border="1" cellpadding="5" cellspacing="0">';
    echo '<tr><th>Название</th><th>Автор</th><th>Уровень вложенности</th></tr>';

    foreach ($data as $row) {
        // Отступ в зависимости от уровня вложенности (если включен режим иерархии)
        $indent = '';
        if ($hierarchy_mode && $row->level > 1) {
            $indent = str_repeat('&nbsp;&nbsp;&nbsp;&nbsp;', $row->level - 1) . '  ';
        }

        // Форматирование заголовка
        $titleFormatted = formatTitle($row->title, $title_case_mode);

        echo '<tr>';
        echo '<td>' . $indent . htmlspecialchars($titleFormatted) . '</td>';
        echo '<td>' . htmlspecialchars($row->name) . '</td>';
        echo '<td>' . htmlspecialchars($row->level) . '</td>';
        echo '</tr>';
    }

    echo '</table>';
} else {
    echo '<p>Нет данных для отображения.</p>';
}
