# -*- coding: utf-8 -*-

import logging


class Error(Exception):
    pass


class ConfigError(Error):
    pass


class NotFoundError(Error):
    pass


class ArgumentError(Error):
    pass


class ParseError(Error):
    pass


logger = logging.getLogger(__name__)

loghandler = logging.StreamHandler()
loghandler.setFormatter(
        logging.Formatter('%(levelname)s %(asctime)s %(filename)s:%(lineno)d: '
                          '%(message)s', '%Y-%m-%d %H:%M:%S'))

logger.addHandler(loghandler)


def set_verbose(verbosity):
    if verbosity == 0:
        logger.setLevel(logging.WARNING)
    elif verbosity == 1:
        logger.setLevel(logging.INFO)
    else:
        logger.setLevel(logging.DEBUG)


set_verbose(False)
