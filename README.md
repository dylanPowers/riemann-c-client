Riemann C client library
========================

This is a C client library for the [Riemann][riemann] monitoring
system. While there is already an [existing library][gkos-riemann],
this one has different goals, and was built from the ground up to suit
a particular need the existing library could not fulfil: an
LGPL-compatible library, with a test suite and a convenient and
straightforward API, and strong compatibility guarantees.

 [riemann]: http://riemann.io/
 [gkos-riemann]: https://github.com/gkos/riemann-c-client

There's still a good mile to go, but progress is being made, and the
library is already usable for some purposes (see the
[demo programs][demos]). Once the library reaches stability, it will
be using [Semantic Versioning][semver], and API and ABI compatibility
will be guaranteed accordingly.

 [demos]: https://github.com/algernon/riemann-c-client/tree/master/src
 [semver]: http://semver.org/

Until then, a trivial program to send a single event to
[Riemann][riemann]:

```c
#include <riemann/client.h>
#include <riemann/event.h>
#include <riemann/message.h>
#include <riemann/attribute.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
main (void)
{
  riemann_client_t *client;
  riemann_message_t *message;
  int e;

  message = riemann_message_create_with_events
   (riemann_event_create (RIEMANN_EVENT_FIELD_HOST, "localhost",
                          RIEMANN_EVENT_FIELD_SERVICE, "demo-client",
                          RIEMANN_EVENT_FIELD_STATE, "ok",
                          RIEMANN_EVENT_FIELD_TAGS, "demo-client", "riemann-c-client", NULL,
                          RIEMANN_EVENT_FIELD_NONE),
    NULL);

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  if (!client)
    {
      fprintf (stderr,  "Error while connecting to Riemann: %s\n",
               strerror (errno));
      exit (EXIT_FAILURE);
    }

  if ((e = riemann_client_send_message (client, message)) != 0)
    {
      fprintf (stderr,  "Error while sending message: %s\n", strerror (-e));
      exit (EXIT_FAILURE);
    }

  riemann_message_free (message);
  riemann_client_free (client);

  return EXIT_SUCCESS;
}
```
