
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `stat`
--

-- --------------------------------------------------------

CREATE TABLE IF NOT EXISTS `ip_geo_records` (
  `start` int(10) unsigned NOT NULL,
  `end` int(10) unsigned NOT NULL,
  `country` varchar(30) NOT NULL,
  `province` varchar(30) NOT NULL,
  `city` varchar(30) NOT NULL,
  `district` varchar(30) NOT NULL,
  `isp` varchar(30) NOT NULL,
  `type` varchar(30) NOT NULL,
  `desc` varchar(30) NOT NULL,
  `ret` tinyint(4) NOT NULL,
  `lat` float NOT NULL,
  `lng` float NOT NULL,
  `manual` tinyint(1) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

ALTER TABLE `ip_geo_records`
  ADD PRIMARY KEY (`start`,`end`),
  ADD KEY `country` (`country`,`province`,`city`),
  ADD KEY `start` (`start`),
  ADD KEY `end` (`end`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
