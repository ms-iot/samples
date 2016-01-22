/* The following functions must be written in ARM mode */
/* these functions are called directly by an exception vector */

/*------------------------------------------------------------------------------ */
/*         Internal functions */
/*------------------------------------------------------------------------------ */
/*------------------------------------------------------------------------------ */
/* Default spurious interrupt handler. Infinite loop. */
/*------------------------------------------------------------------------------ */
void AT91F_Spurious_handler(
    void)
{
    while (1);
}

/*------------------------------------------------------------------------------ */
/* Default handler for fast interrupt requests. Infinite loop. */
/*------------------------------------------------------------------------------ */
void AT91F_Default_FIQ_handler(
    void)
{
    while (1);
}

/*------------------------------------------------------------------------------ */
/* Default handler for standard interrupt requests. Infinite loop. */
/*------------------------------------------------------------------------------ */
void AT91F_Default_IRQ_handler(
    void)
{
    while (1);
}
