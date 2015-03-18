#include <php.h>
#include <zend_exceptions.h>
#include "../php_cassandra.h"
#include "util/math.h"
#include "decimal.h"

extern zend_class_entry *cassandra_ce_InvalidArgumentException;

zend_class_entry *cassandra_ce_Decimal = NULL;

/* {{{ Cassandra\Decimal::__construct(string) */
PHP_METHOD(CassandraDecimal, __construct)
{
  char *value;
  int value_len;
  int scale = 0;
  cassandra_decimal* number;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &value, &value_len) == FAILURE) {
    return;
  }

  number = (cassandra_decimal*) zend_object_store_get_object(getThis() TSRMLS_CC);

  if (!php_cassandra_parse_decimal(value, value_len, &number->value, &scale))
    return;

  number->scale = scale;

  char* string;
  int string_len;
  php_cassandra_format_integer(number->value, &string, &string_len);

  zend_update_property_stringl(cassandra_ce_Decimal, getThis(), "value", strlen("value"), string, string_len TSRMLS_CC);
  zend_update_property_long(cassandra_ce_Decimal, getThis(), "scale", strlen("scale"), (long) number->scale TSRMLS_CC);
  efree(string);
}
/* }}} */

/* {{{ Cassandra\Decimal::__toString() */
PHP_METHOD(CassandraDecimal, __toString)
{
  // zval *zode = zend_read_property(cassandra_ce_Decimal, getThis(), "value", strlen("value"), 0 TSRMLS_CC);
  // number = (cassandra_decimal*) zend_object_store_get_object(getThis() TSRMLS_CC);
  //
  // char* string = emalloc((Z_STRLEN_P(value) + 1) * sizeof(char));
  //
  // if (number->scale > 0) {
  //   memcpy(string, Z_STRVAL_P(value), Z_STRLEN_P(value) - number->scale);
  //   memcpy(&string[number->scale], &(Z_STRVAL_P(value)[number->scale + 1]), Z_STRLEN_P(value) - number->scale - 1);
  // } else {
  //   memcpy(string, Z_STRVAL_P(value), Z_STRLEN_P(value));
  // }
  //
  // RETURN_STRING(Z_STRVAL_P(string), 1);
  zval *zode = zend_read_property(cassandra_ce_Decimal, getThis(), "value", strlen("value"), 0 TSRMLS_CC);

  RETURN_STRING(Z_STRVAL_P(zode), 1);
}
/* }}} */

/* {{{ Cassandra\Decimal::value() */
PHP_METHOD(CassandraDecimal, value)
{
  zval *zode = zend_read_property(cassandra_ce_Decimal, getThis(), "value", strlen("value"), 0 TSRMLS_CC);

  RETURN_STRING(Z_STRVAL_P(zode), 1);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo___construct, 0, ZEND_RETURN_VALUE, 1)
  ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static zend_function_entry CassandraDecimal_methods[] = {
  PHP_ME(CassandraDecimal, __construct, arginfo___construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
  PHP_ME(CassandraDecimal, __toString, NULL, ZEND_ACC_PUBLIC)
  PHP_ME(CassandraDecimal, value, NULL, ZEND_ACC_PUBLIC)
  PHP_FE_END
};

static void
php_cassandra_decimal_free(void *object TSRMLS_DC)
{
  cassandra_decimal* number = (cassandra_decimal*) object;

  mpz_clear(number->value);
  zend_object_std_dtor(&number->zval TSRMLS_CC);

  efree(number);
}

static zend_object_value
php_cassandra_decimal_new(zend_class_entry* class_type TSRMLS_DC)
{
  zend_object_value retval;
  cassandra_decimal *number;

  number = (cassandra_decimal*) emalloc(sizeof(cassandra_decimal));
  memset(number, 0, sizeof(cassandra_decimal));

  mpz_init(number->value);
  zend_object_std_init(&number->zval, class_type TSRMLS_CC);
  object_properties_init(&number->zval, class_type TSRMLS_CC);
  number->scale = 0;

  retval.handle = zend_objects_store_put(number, (zend_objects_store_dtor_t) zend_objects_destroy_object, php_cassandra_decimal_free, NULL TSRMLS_CC);
  retval.handlers = zend_get_std_object_handlers();

  return retval;
}

void cassandra_define_CassandraDecimal(TSRMLS_D)
{
  zend_class_entry ce;

  INIT_CLASS_ENTRY(ce, "Cassandra\\Decimal", CassandraDecimal_methods);
  cassandra_ce_Decimal = zend_register_internal_class(&ce TSRMLS_CC);
  cassandra_ce_Decimal->ce_flags |= ZEND_ACC_FINAL_CLASS;
  cassandra_ce_Decimal->create_object = php_cassandra_decimal_new;

  /* fields */
  zend_declare_property_string(cassandra_ce_Decimal, "value", strlen("value"), "", ZEND_ACC_PRIVATE TSRMLS_CC);
  zend_declare_property_long(cassandra_ce_Decimal, "scale", strlen("scale"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
}
