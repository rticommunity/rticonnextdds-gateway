.. include:: vars.rst

.. _section-troubleshooting:

***************
Troubleshooting
***************

This chapter provides solutions to common issues when working 
with |DIA|.

ODBC Driver Issues
==================

**Problem**: Cannot connect to database or "ODBC driver not found" errors

**Solutions**:

1. **Verify ODBC driver installation**:

   .. code-block:: console

       # List installed ODBC drivers
       odbcinst -q -d

2. **Check DSN configuration**:

   .. code-block:: console

       # Test ODBC connection
       isql -v <DSN_NAME>

3. **Verify odbc.ini location**: ODBC typically looks for configuration in:
   
   - ``/etc/odbc.ini`` (system-wide)
   - ``~/.odbc.ini`` (user-specific)
   - Set ``ODBCINI`` environment variable to specify location

4. **Check driver paths**: Ensure the ODBC driver libraries are in the library path:

   .. code-block:: console

       export LD_LIBRARY_PATH=/path/to/odbc/drivers:$LD_LIBRARY_PATH

General Database Connection Issues
==================================

**Problem**: "Table already exists" errors on startup

**Solution**: If ``drop_table_on_dispose`` was set to ``false``, tables from previous
runs may exist. Either:

- Drop tables manually before restarting
- Set ``drop_table_on_dispose`` to ``true`` for development/testing

**Problem**: Poor performance with large datasets

**Solution**: 

- Add indexes on timestamp columns for time-based queries
- Use ``instance_history`` to limit table growth
- Consider database-specific optimizations (partitioning, connection pooling)
